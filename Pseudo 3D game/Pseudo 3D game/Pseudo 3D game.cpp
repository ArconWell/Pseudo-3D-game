#include <iostream>
#include <Windows.h>
#include <chrono>

using namespace std;

int screenWidth = 120;//ширина консоли
int screenHeight = 40;//высота консоли

float playerX = 1.0f;//координата игрока по x
float playerY = 1.0f;//координата игрока по y
float playerA = 0.0f;//направление игрока

int mapHeight = 16;//высота игрового поля
int mapWidth = 16;//ширина игрового поля

float FOV = 3.14159 / 3;//угол обзора (поле видимости)
float depth = 30.0f;//максимальная дистанция обзора

auto tp1 = chrono::system_clock::now(); // Переменные для подсчета
auto tp2 = chrono::system_clock::now(); // пройденного времени

int main()
{
	//7 строк ниже представляют собой конструкцию, выводящую данные в консоль. Cout не используем, так как он слишком медленный (без понятия как эта конструкция работает)
	wchar_t* screen = new wchar_t[screenWidth * screenHeight + 1]; // Массив для записи в буфер
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); // Буфер экрана
	SetConsoleActiveScreenBuffer(hConsole); // Настройка консоли
	DWORD dwBytesWritten = 0; // Для дебага

	screen[screenWidth * screenHeight] = '\0';  // Последний символ - окончание строки
	WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0, 0 }, &dwBytesWritten); // Запись в буфер

	wstring map;//wstring - "широкие", локализованные символы
	map += L"################";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	while (1)//цикл игры
	{
		tp2 = chrono::system_clock::now();//в этих 4 строках рассчитываем время цикла
		chrono::duration <float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)//клавишей W идём вперёд
		{
			if (map[(int)playerY * mapWidth + (int)playerX] != '#')//если ячейка, куда хотим переместиться не является стеной
			{
				playerX += sinf(playerA);//sinf точнее sin
				playerY += cosf(playerA);
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)//клавишей S идём назад
		{
			if (map[(int)playerY * mapWidth + (int)playerX] != '#')
			{
				playerX -= sinf(playerA);
				playerY -= cosf(playerA);
			}
		}

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)//клавишей A поворачиваем по часовой стрелке
		{
			playerA -= 1.5f * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)//клавишей D поворачиваем против часовой стрелке
		{
			playerA += 1.5f * fElapsedTime;
		}
	}
	return 0;
}