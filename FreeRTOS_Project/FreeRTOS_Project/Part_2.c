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

TaskHandle_t tkHandler1 = NULL;
TaskHandle_t manualSpeedHandler = NULL;
TaskHandle_t autoSpeedHandler = NULL;
TaskHandle_t gearDisplayHandler = NULL;
TaskHandle_t gearCalcHandler = NULL;
SemaphoreHandle_t speedSemaphore = NULL;

#pragma warning(disable:4996)

// char arrays to display logos
char line0[256] = { ' ', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', ' ', ' ', ' ', ' ' };
char line1[256] = { '/', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\\', ' ', ' ', ' ' };
char line2[256] = { '|', ' ', ' ', 'J', 'a', 'g', 'u', 'a', 'r', ' ', ' ', '|', ' ', ' ', ' ', ' ' };
char line3[256] = { '\\', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '/', ' ', ' ', ' ' };

char line4[256] = { ' ', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', ' ', ' ', ' ', ' ' };
char line5[256] = { '/', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\\', ' ', ' ', ' ' };
char line6[256] = { '|', 'L', 'a', 'n', 'd', ' ', 'R', 'o', 'v', 'e', 'r', '|', ' ', ' ', ' ', ' ' };
char line7[256] = { '\\', '_', '_', '_', '_', '_', '_', '_', '_', '_', '_', '/', ' ', ' ', ' ' };

// global variables
struct tm day_time;
int currentTemp;
int currentPressure;
int currentHumidity;
int currentSpeed;
int command;
int currentGear = 1;
int nextGear = 1;
float rateOfChange;

void taskGetRandomWeather(void* pvParameter) {
	// infinite loop
	while (1) {
		// current temp = random int between 0 and 40
		currentTemp = rand() % 40;
		// current pressure is random int between 900 and 1200
		currentPressure = 900 + rand() % 300;
		// current humidity is random int between 0 and 100
		currentHumidity = rand() % 100;
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}

// function to simulate engine braking i.e. speed decreasing when not on accelerator
void taskSimEngineBraking(void* pvParameters) {
	while (1) {
		if (command == 0) { 
			if (speedSemaphore != NULL && currentSpeed > 0) {
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if (xSemaphoreTake(speedSemaphore, (TickType_t)500) == pdTRUE) {
					currentSpeed--;
					xSemaphoreGive(speedSemaphore);
				}
				else {
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
					xSemaphoreGive(speedSemaphore);
				}
			}
		}
		vTaskDelay(350 / portTICK_PERIOD_MS);
	}
}

void taskEventManualSpeed(void* pvParameters) {
	// init char array to display the chosen metric
	// infinite loop
	while (1) {
		// get input from user
		command = getch();
		// if commmand is to accelerate
		if (command == 'w') {
			if (speedSemaphore != NULL) {
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if (xSemaphoreTake(speedSemaphore, (TickType_t)100) == pdTRUE) {
					// increment speed
					currentSpeed++;
					// return semaphore
					xSemaphoreGive(speedSemaphore);
					// reset command
					command = 0;
				}
				else {
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
					// reset command
					command = 0;
					// return semaphore
					xSemaphoreGive(speedSemaphore);
				}
			}
		}
		// if command is decelerate
		else if (command == 's' && currentSpeed > 0) {
			if (speedSemaphore != NULL) {
				/* See if we can obtain the semaphore.  If the semaphore is not
				available wait 10 ticks to see if it becomes free. */
				if (xSemaphoreTake(speedSemaphore, (TickType_t)100) == pdTRUE) {
					// decrement speed
					currentSpeed--;
					// return semaphore
					xSemaphoreGive(speedSemaphore);
					// reset command
					command = 0;
				}
				else {
					/* We could not obtain the semaphore and can therefore not access
					the shared resource safely. */
					// reset command
					command = 0;
					// return semaphore
					xSemaphoreGive(speedSemaphore);
				}
			}
		}
		else {
			command = 0;
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void taskDisplaySpeed(void* pvParameters) {
	// task to display speed
	// temp char array
	char temp[256];
	while (1) {
		// continuously display speed from shared resource currentSpeed
		sprintf_s(temp, 255, "%d MPH   \n", currentSpeed);
		printXY(50, 10, temp);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void taskPeriodicDisplayTemp(void* pvParameters) {
	// init char array
	char temp[256];
	// infinite loop
	while (1) {
		sprintf_s(temp, 255, "%d%cC   \n", currentTemp, 223);
		printXY(1, 2, temp);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
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
		printXY(1, 1, temp);
		// delay
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(tkHandler1);
}

void taskPeriodicDisplayDate(void* pvParameters) {
	// initialise char array to display date
	char temp[256];
	// infinite loop
	while (1) {
		// create string to print to screen, get day, month and add 1 as it runs 0-11 not 1-12 and get year but add 1900 as it calculates years since then
		sprintf_s(temp, 255, "%d/%d/%d\n", day_time.tm_mday, day_time.tm_mon + 1, day_time.tm_year + 1900);
		// print to (10,11)
		printXY(110, 1, temp);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void taskGearChange(void* pvParameters) {
	char temp[256];
	while (1) {
		if (currentSpeed == 10 || currentSpeed == 30 || currentSpeed == 40 || currentSpeed == 50 || currentSpeed == 60 || currentSpeed == 70) {
			if (rateOfChange > 0) {
				nextGear = currentGear + 1;
			}
			else if (rateOfChange < 0) {
				nextGear = currentGear - 1;
			}
			vTaskSuspend(manualSpeedHandler);
			vTaskSuspend(autoSpeedHandler);
			vTaskSuspend(gearDisplayHandler);
			sprintf_s(temp, 255, "Gear: %d -> %d\n", currentGear, nextGear);
			currentGear = nextGear;
			printXY(48, 11, temp);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			sprintf_s(temp, 255, "Gear: %d     \n", currentGear);
			printXY(48, 11, temp);
			if (rateOfChange > 0) {
				currentSpeed++;
			}
			else if (rateOfChange < 0) {
				currentSpeed--;
			}
			vTaskResume(manualSpeedHandler);
			vTaskResume(autoSpeedHandler);
			vTaskResume(gearDisplayHandler);
		}
	}
}

void taskCalculateAcceleration(void* pvParameters) {
	float previous = 0;
	float current = 0;
	//int rateOfChange;
	while (1) {
		previous = current;
		current = currentSpeed;
		rateOfChange = (current - previous) / 10;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void taskDisplayGear(void* pvParameters) {
	char temp[256];
	// infinite loop
	while (1) {
		sprintf_s(temp, 255, "Gear: %d     \n", currentGear);
		// print to (10,11)
		printXY(48, 11, temp);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

// task to display logo using global char arrays
void taskDisplayLogo(void* pvParameters) {
	printXY(1, 25, line0);
	printXY(1, 26, line1);
	printXY(1, 27, line2);
	printXY(1, 28, line3);
	printXY(15, 25, line4);
	printXY(15, 26, line5);
	printXY(15, 27, line6);
	printXY(15, 28, line7);
}

int main(void) {
	speedSemaphore = xSemaphoreCreateMutex();
	xTaskCreate(taskDisplayLogo, "Display_Logo", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskGetRandomWeather, "Get_Weather", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskCalculateDateTime, "Calc_Date_Time", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayTime, "Display_Time", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &tkHandler1);
	xTaskCreate(taskPeriodicDisplayDate, "Display_Date", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskPeriodicDisplayTemp, "Display_Temp", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskEventManualSpeed, "Control_Speed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &manualSpeedHandler);
	xTaskCreate(taskDisplaySpeed, "Display_Speed", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskSimEngineBraking, "Control_Engine_Braking", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &autoSpeedHandler);
	xTaskCreate(taskDisplayGear, "Display_Gear", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &gearDisplayHandler);
	xTaskCreate(taskGearChange, "Change_Gear", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	xTaskCreate(taskCalculateAcceleration, "Calc_Acc", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
	vTaskStartScheduler();
	for (;;);
}