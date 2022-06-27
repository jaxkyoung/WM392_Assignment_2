#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include <string.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "WM392.h"
#include "semphr.h"
//#include <curl/curl.h>

TaskHandle_t tkHandler1 = NULL;

// https://api.openweathermap.org/data/2.5/weather?lat=52.3793&lon=-1.5615&appid=e883ab104dd0964c22cc9e93ae7734be API call for weather at Warwick Uni

#pragma warning(disable:4996)

struct tm day_time;
int currentTemp;
int currentPressure;
int currentHumidity;

void taskGetRandomWeather(void* pvParameters) {
	// infinite loop
	while (1) {
		// current temp = random int between 0 and 40
		currentTemp = rand() % 40;
		// current pressure is random int between 900 and 1200
		currentPressure = 900 + rand() % 300;
		// current humidity is random int between 0 and 100
		currentHumidity = rand() % 100;
	}
}

/*void taskGetAPIWeather(void* pvParameters) {
	CURL* hnd = curl_easy_init();

	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(hnd, CURLOPT_URL, "https://community-open-weather-map.p.rapidapi.com/weather?q=Warwick%2Cuk&lat=0&lon=0&callback=test&id=2172797&lang=null&units=metric&mode=xml");

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "X-RapidAPI-Host: community-open-weather-map.p.rapidapi.com");
	headers = curl_slist_append(headers, "X-RapidAPI-Key: 2172abc47bmshadab69c37af5e1ap181d88jsnb47e5db0a501");
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);

	CURLcode ret = curl_easy_perform(hnd);
	curl_easy_cleanup(hnd);
}*/

void taskEventManualDisplay(void* pvParameters) {
	// init char array to display the chosen metric
	char temp[256];
	// command variable to be input by user using keyboard T H or P
	int command;
	// infinite loop
	while (1) {
		// get input from user
		command = getch();
		// switch case statement - from T, H, P
		switch (command) {
		case 'T':
		case 't':
			// if T, then print currentTemp for 2 seconds
			sprintf_s(temp, 255, "Temp = %d%cC\n", currentTemp, 223);
			printXY(10, 12, temp);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
			break;
		case 'H':
		case 'h':
			// if H, then print currentHumidity for 2 seconds
			sprintf_s(temp, 255, "Humidity = %d%%\n", currentHumidity);
			printXY(10, 12, temp);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
			break;
		case 'P':
		case 'p':
			// if P, then print currentPRessure for 2 seconds
			sprintf_s(temp, 255, "Pressure = %dhPA\n", currentPressure);
			printXY(10, 12, temp);
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
			break;
		}
	}
}

void taskPeriodicDisplayTemp(void* pvParameters) {
	// init char array
	char temp[256];
	// infinite loop
	while (1) {
		// if seconds == 10 then display temp
		if (day_time.tm_sec == 10) {
			sprintf_s(temp, 255, "Temp = %d%cC\n", currentTemp, 223);
			printXY(10, 12, temp);
			// wait 2 seconds
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
		}
	}
}

void taskPeriodicDisplayPressure(void* pvParameters) {
	// init char array
	char temp[256];
	// infinite loop
	while (1) {
		// if seconds == 50 then display pressure
		if (day_time.tm_sec == 50) {
			sprintf_s(temp, 255, "Pressure = %dhPA\n", currentPressure);
			printXY(10, 12, temp);
			// wait 2 seconds
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
		}
	}
}

void taskPeriodicDisplayHumidity(void* pvParameters) {
	// init char array
	char temp[256];
	// infinite loop
	while (1) {
		// if seconds == 30 then display humidity
		if (day_time.tm_sec == 30) {
			sprintf_s(temp, 255, "Humidity = %d%%\n", currentHumidity);
			printXY(10, 12, temp);
			// wait 2 seconds
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 12, temp);
		}
	}
}

void taskCalculateDateTime(void* pvParameters) {
	// initialise t variable of time type
	time_t t;
	// infinite loop
	while (1) {
		// get time once every second
		time(&t);
		// calculate local time and save to day time global struct
		localtime_s(&day_time, &t);
		// delay
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void taskPeriodicDisplayTime(void* pvParameters) {
	// initialise char array to display time
	char temp[256];
	// infinite loop
	while (1) {
		// create string to print to screen, get hour, minute, and second and print with 2 sig figs
		sprintf_s(temp, 255, "%02d:%02d:%02d\n", day_time.tm_hour, day_time.tm_min, day_time.tm_sec);
		// print to (10,10)
		printXY(10, 10, temp);
		// delay
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(tkHandler1);
}

const int digits[10][7] = {
	//    A, B, C, D, E, F, G
	/**/{ 1, 1, 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 0, 0, 0, 0 },
		{ 1, 1, 0, 1, 1, 0, 1 },
		{ 1, 1, 1, 1, 0, 0, 1 },
		{ 0, 1, 1, 0, 0, 1, 1 },
		{ 1, 0, 1, 1, 0, 1, 1 },
		{ 1, 0, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 0, 1, 1 }
};

// define char array to show LED digits
char digit1[3][256] = { ' ' };
char digit2[3][256] = { ' ' };
char digit3[3][256] = { ' ' };
char digit4[3][256] = { ' ' };
char digit5[3][256] = { ' ' };
char digit6[3][256] = { ' ' };

void popDigit(int digit, int position) {
	// define output char array
	char out[3][256] = { ' ' };
	// for each segment
	for (int i = 0; i < 7; i++) {
		// check if requested digit needs the selected segment
		if (digits[digit][i] == 1) /* Has digit the i segment? */
			switch (i) {
			// fill positions in out char array
			case 0: out[0][1] = '_'; break; //A
			case 1: out[1][2] = '|'; break; //B
			case 2: out[2][2] = '|'; break; //C
			case 3: out[2][1] = '_'; break; //D
			case 4: out[2][0] = '|'; break; //E
			case 5: out[1][0] = '|'; break; //F
			case 6: out[1][1] = '_'; break; //G
			}
	}
	// check position requested and fill corresponding digit array
	switch (position) {
		// if position
		case 1:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit1[i], out[i]);
			}
			break;
		// if position
		case 2:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit2[i], out[i]);
			}
			break;
		// if position
		case 3:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit3[i], out[i]);
			}
			break;
		// if position
		case 4:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit4[i], out[i]);
			}
			break;
		// if position
		case 5:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit5[i], out[i]);
			}
			break;
		// if position
		case 6:
			// for each line in the out char array copy into the digit array 
			for (int i = 0; i < 3; i++) {
				strcpy(digit6[i], out[i]);
			}
			break;
	}
}

void taskPeriodicDisplayLEDTime(void* pvParameter) {
	// define padding char array
	char padding[256];
	// print a line of underscores to surround the LED display - this only needs to be printed once
	sprintf_s(padding, 255, "______________________________\n");
	printXY(10, 5, padding);
	printXY(10, 9, padding);
	// infinite loop
	while (1) {
		// empty output digits char array
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 200; j++) {
				digit1[i][j] = ' ';
				digit2[i][j] = ' ';
				digit3[i][j] = ' ';
				digit4[i][j] = ' ';
				digit5[i][j] = ' ';
				digit6[i][j] = ' ';
			}
		}
		// get single digits from current time
		int hourDig1;
		hourDig1 = day_time.tm_hour / 10;
		// populate LED digit with the digit from above and put it in the correct position
		// for this example, it is the first hour digit so would be position 1/6
		// repeat this process for the rest of the digits
		popDigit(hourDig1, 1);
		int hourDig2;
		hourDig2 = day_time.tm_hour % 10;
		popDigit(hourDig2, 2);
		int minDig1;
		minDig1 = day_time.tm_min / 10;
		popDigit(minDig1, 3);
		int minDig2;
		minDig2 = day_time.tm_min % 10;
		popDigit(minDig2, 4);
		int secDig1;
		secDig1 = day_time.tm_sec / 10;
		popDigit(secDig1, 5);
		int secDig2;
		secDig2 = day_time.tm_sec % 10;
		popDigit(secDig2, 6);

		// print the digits to the screen, i.e. position 0,1, and 2 of the digit char arrays
		printXY(10, 6, digit1[0]);
		printXY(10, 7, digit1[1]);
		printXY(10, 8, digit1[2]);
		printXY(13, 6, digit2[0]);
		printXY(13, 7, digit2[1]);
		printXY(13, 8, digit2[2]);
		// print gap between hour and minute
		printXY(16, 7, "-");
		printXY(16, 8, "-");
		printXY(17, 6, digit3[0]);
		printXY(17, 7, digit3[1]);
		printXY(17, 8, digit3[2]);
		printXY(20, 6, digit4[0]);
		printXY(20, 7, digit4[1]);
		printXY(20, 8, digit4[2]);
		// print gap between minute and second
		printXY(23, 7, "-");
		printXY(23, 8, "-");
		printXY(24, 6, digit5[0]);
		printXY(24, 7, digit5[1]);
		printXY(24, 8, digit5[2]);
		printXY(27, 6, digit6[0]);
		printXY(27, 7, digit6[1]);
		printXY(27, 8, digit6[2]);
		// delay 1 second
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void taskPeriodicDisplayDate(void* pvParameters) {
	// initialise char array to display date
	char temp[256];
	// infinite loop
	while (1) {
		// if seconds == 00 then print date
		if (day_time.tm_sec == 0) {
			// create string to print to screen, get day, month and add 1 as it runs 0-11 not 1-12 and get year but add 1900 as it calculates years since then
			sprintf_s(temp, 255, "%d/%d/%d\n", day_time.tm_mday, day_time.tm_mon + 1, day_time.tm_year + 1900);
			// print to (10,11)
			printXY(10, 11, temp);
			// wait 2 seconds
			vTaskDelay(2000 / portTICK_PERIOD_MS);
			// print empty string to (10,11) to clear date
			sprintf_s(temp, 255, "                          \n");
			printXY(10, 11, temp);
		}
	}
}

int main(void) {
	xTaskCreate(taskPeriodicDisplayLEDTime, "Display_LED", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskGetRandomWeather, "Get_Weather", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskCalculateDateTime, "Calc_Date_Time", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayTime, "Display_Time", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &tkHandler1);
	xTaskCreate(taskPeriodicDisplayDate, "Display_Date", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayTemp, "Display_Temp", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayHumidity, "Display_Humidity", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayPressure, "Display_Pressure", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskEventManualDisplay, "Display_Pressure", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	vTaskStartScheduler();
	for (;;);
}