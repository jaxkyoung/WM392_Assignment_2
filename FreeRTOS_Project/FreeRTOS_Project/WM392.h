#pragma once
void clearScreen() {
	printf("\033[2J\033[H");
}

void printXY(int x, int y, char* str) {
	COORD c;
	c.X = x;
	c.Y = y;
	SetConsoleCursorPosition(
		GetStdHandle(STD_OUTPUT_HANDLE), c);
	printf("%s", str);
}
