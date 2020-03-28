#include <iostream>
#include <Windows.h>
#include <chrono>
#include <vector>
#include <algorithm>

using namespace std;

int screenWidth = 40;// ширина консоли
int screenHeight = 40;//высота консоли

float playerX = 1.0f;//координата игрока по x
float playerY = 1.0f;//координата игрока по y
float playerA = 0.0f;//направление игрока

int mapHeight = 16;//высота игрового поля
int mapWidth = 16;//ширина игрового поля

float FOV = 3.14159 / 4.0f;//угол обзора (поле видимости)
float depth = 16.0f;//максимальная дистанция обзора
float speed = 3.0f;
int main()
{
	//7 строк ниже представляют собой конструкцию, выводящую данные в консоль. Cout не используем, так как он слишком медленный (без понятия как эта конструкция работает)
	wchar_t* screen = new wchar_t[screenWidth * screenHeight + 1]; // Массив для записи в буфер
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); // Буфер экрана
	SetConsoleActiveScreenBuffer(hConsole); // Настройка консоли
	DWORD dwBytesWritten = 0; // Для дебага

	wstring map;//wstring - "широкие", локализованные символы
	map += L"################";
	map += L"#.........#....#";
	map += L"########..#....#";
	map += L"#..............#";
	map += L"#.####.....#####";
	map += L"#..............#";
	map += L"#.....##......##";
	map += L"#..............#";
	map += L"#......###..####";
	map += L"#......#.......#";
	map += L"###....#.....###";
	map += L"#......#.......#";
	map += L"#......#.....#.#";
	map += L"#..##..#.......#";
	map += L"#......#.......#";
	map += L"################";

	auto tp1 = chrono::system_clock::now(); // Переменные для подсчета
	auto tp2 = chrono::system_clock::now(); // пройденного времени

	while (1)//цикл игры
	{
		// We'll need time differential per frame to calculate modification
		// to movement speeds, to ensure consistant movement, as ray-tracing
		// is non-deterministi
		tp2 = chrono::system_clock::now();//в этих 4 строках рассчитываем время цикла
		chrono::duration <float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)//клавишей A поворачиваем по часовой стрелке
		{
			playerA -= (speed * 0.75f) * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)//клавишей D поворачиваем против часовой стрелке
		{
			playerA += (speed * 0.75f) * fElapsedTime;
		}

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)//клавишей W идём вперёд
		{
			playerX += sinf(playerA) * speed * fElapsedTime;//sinf точнее sin
			playerY += cosf(playerA) * speed * fElapsedTime;
			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')//если ячейка, куда хотим переместиться не является стеной
			{
				playerX -= sinf(playerA) * speed * fElapsedTime;//sinf точнее sin
				playerY -= cosf(playerA) * speed * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)//клавишей S идём назад
		{
			playerX -= sinf(playerA) * speed * fElapsedTime;
			playerY -= cosf(playerA) * speed * fElapsedTime;
			if (map.c_str()[(int)playerX * mapWidth + (int)playerY] == '#')
			{
				playerX += sinf(playerA) * speed * fElapsedTime;
				playerY += cosf(playerA) * speed * fElapsedTime;
			}
		}

		for (int x = 0; x < screenWidth; x++)//проходим по всем x
		{
			float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)screenWidth) * FOV;//направление луча	
			//находим расстоние до стены в направлении rayAngle

			float distanceToWall = 0.0f;//расстоние до препятствия в направлении rayAngle
			bool hitWall = false;//достигнул ли луч стенки
			bool bBoundary = false;		// Set when ray hits boundary between two wall blocks

			float eyeX = sinf(rayAngle);//координаты единичного вектора rayAngle
			float eyeY = cosf(rayAngle);

			while (!hitWall && distanceToWall < depth)//пока не столкнулись со стеной или не вышли за радиус видимости
			{
				distanceToWall += 0.1f;

				int testX = (int)(playerX + eyeX * distanceToWall);//точка на игровом поле
				int testY = (int)(playerY + eyeY * distanceToWall);//в которую попал луч

				if (testX < 0 || testX >= mapWidth || testY < 0 || testY >= mapHeight)//если вышли за зону
				{
					hitWall = true;
					distanceToWall = depth;//в этом случае стена для программы есть, но мы сделаем её прозрачного цвета, поэтому пользовател её не увидит
				}
				else
				{
					//TODO разобраться в этом участке кода

					// Ray is inbounds so test to see if the ray cell is a wall block
					if (map.c_str()[testX * mapWidth + testY] == '#')
					{
						// Ray has hit wall
						hitWall = true;

						// To highlight tile boundaries, cast a ray from each corner
						// of the tile, to the player. The more coincident this ray
						// is to the rendering ray, the closer we are to a tile 
						// boundary, which we'll shade to add detail to the walls
						vector<pair<float, float>> p;

						// Test each corner of hit tile, storing the distance from
						// the player, and the calculated dot product of the two rays
						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Angle of corner to eye
								float vy = (float)testY + ty - playerY;
								float vx = (float)testX + tx - playerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (eyeX * vx / d) + (eyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}

						// Sort Pairs from closest to farthest
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						// First two/three are closest (we will never see all four)
						float fBound = 0.01f;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			//для отрисовки вертикальной полоски зададим две координаты по Y: первая - координата, где заканчивается небо и 
			//начинается стена, а вторая - координата, где заканчивается стена и начинается пол
			int ceiling = (float)(screenHeight / 2.0) - screenHeight / ((float)distanceToWall);
			int floor = screenHeight - ceiling;

			short shade;

			if (distanceToWall <= depth / 4.0f)        shade = 0x2588; // Если стенка близко, то рисуем 
			else if (distanceToWall < depth / 3.0f)    shade = 0x2593; // светлую полоску
			else if (distanceToWall < depth / 2.0f)    shade = 0x2592; // Для отдалённых участков 
			else if (distanceToWall < depth)           shade = 0x2591; // рисуем более темную.
			else                                         shade = ' ';//Этот, если стенка далеко

			if (bBoundary)		shade = '|'; // Black it out

			for (int y = 0; y < screenHeight; y++)//при заданном x проходим по всем y 
			{
				if (y <= ceiling)
				{
					screen[y * screenWidth + x] = ' ';
				}
				else if (y > ceiling && y <= floor)
				{
					screen[y * screenHeight + x] = shade;
				}
				else//пол
				{
					//с полом делаем аналогично со стенками - более близкие части рисуем более заметными символами
					float b = 1.0f - ((float)y - screenHeight / 2.0f) / ((float)screenHeight / 2.0f);
					if (b < 0.25)        shade = '#';
					else if (b < 0.5)    shade = 'x';
					else if (b < 0.75)   shade = '.';
					else if (b < 0.9)    shade = '-';
					else                 shade = ' ';

					screen[y * screenHeight + x] = shade;
				}
			}
		}

		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", playerX, playerY, playerA, 1.0f / fElapsedTime);//отображаю статистику

		for (int nx = 0; nx < mapWidth; nx++)//отображаю карту
			for (int ny = 0; ny < mapWidth; ny++)
			{
				screen[(ny + 1) * screenWidth + nx] = map[ny * mapWidth + nx];
			}
		screen[((int)playerX + 1) * screenWidth + (int)playerY] = 'P';

		screen[screenWidth * screenHeight - 1] = '\0';  // Последний символ - окончание строки
		WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0, 0 }, &dwBytesWritten); // Запись в буфер
	}
	return 0;
}