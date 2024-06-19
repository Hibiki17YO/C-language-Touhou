#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<graphics.h>
#include<time.h>
#include<Windows.h>
#include<mmsystem.h>
#include <string.h>
#pragma comment(lib,"WINMM.LIB")
#define MAX_SCORES 10

typedef struct {
	int score;
	time_t timestamp;
} Score;

Score topScores[MAX_SCORES];

enum MyEnum
{
	Width = 420,
	Height = 630,
	BulletNum = 40,
	EnemyNum = 100,
	Large,
	Small,
	SmallN,
	SmallNL,
	SmallNR,
	SmallM,
	SmallML,
	SmallMR
};

IMAGE background;
IMAGE playerReimu[2];
IMAGE playerBullet[2];
IMAGE enemyBullet[2];
IMAGE enemyElf[3][2];

//全局变量分数
int score;

struct character
{
	int x;
	int y;
	bool exist;
	int HP;
	int type;

	//设置有效体积
	int width;
	int height;
}player, enemy[EnemyNum], bullet[BulletNum], enemyBullets[EnemyNum][5];

void loadImg()
{
	loadimage(&background, "./images/bg.png");
	loadimage(&playerReimu[0], "./images/Reimu2.png");
	loadimage(&playerReimu[1], "./images/Reimu1.png");
	loadimage(&playerBullet[0], "./images/pBullet2.png");
	loadimage(&playerBullet[1], "./images/pBullet1.png");
	loadimage(&enemyElf[0][0], "./images/ElfSn2.png");
	loadimage(&enemyElf[0][1], "./images/ElfSn1.png");
	loadimage(&enemyElf[1][0], "./images/ElfSm2.png");
	loadimage(&enemyElf[1][1], "./images/ElfSm1.png");
	loadimage(&enemyElf[2][0], "./images/ElfL2.png");
	loadimage(&enemyElf[2][1], "./images/ElfL1.png");
	loadimage(&enemyBullet[0], "./images/enemyBullet2.png");
	loadimage(&enemyBullet[1], "./images/enemyBullet1.png");
}

void loadGame()
{
	loadImg();
	putimage(0, 0, &background);

	if (player.HP > 0)
	{
		player.exist = true;
	}
	if (player.HP <= 0)
	{
		player.exist = false;
		//mciSendString(_T("close bgm1.mp3"), NULL, 0, NULL);
	}
	if (player.exist)
	{
		putimage(player.x, player.y, &playerReimu[0], SRCAND);
		putimage(player.x, player.y, &playerReimu[1], SRCPAINT);
	}


	for (int i = 0; i < BulletNum; i++)//载入Bullet
	{
		if (bullet[i].exist)
		{
			putimage(bullet[i].x, bullet[i].y, &playerBullet[0], SRCAND);
			putimage(bullet[i].x, bullet[i].y, &playerBullet[1], SRCPAINT);
		}
	}

	for (int j = 0; j < EnemyNum; j++)//载入Elf
	{
		if (enemy[j].exist)
		{
			if (enemy[j].type == Large)
			{
				putimage(enemy[j].x, enemy[j].y, &enemyElf[2][0], SRCAND);
				putimage(enemy[j].x, enemy[j].y, &enemyElf[2][1], SRCPAINT);
			}
			else if (enemy[j].type == SmallNL || enemy[j].type == SmallNR)
			{
				putimage(enemy[j].x, enemy[j].y, &enemyElf[1][0], SRCAND);
				putimage(enemy[j].x, enemy[j].y, &enemyElf[1][1], SRCPAINT);
			}
			else {
				putimage(enemy[j].x, enemy[j].y, &enemyElf[0][0], SRCAND);
				putimage(enemy[j].x, enemy[j].y, &enemyElf[0][1], SRCPAINT);
			}
		}
	}

	for (int i = 0; i < EnemyNum; i++)//载入enemyBullet
	{
		for (int k = 0; k < 5; k++)
			if (enemyBullets[i][k].exist)
			{
				putimage(enemyBullets[i][k].x, enemyBullets[i][k].y, &enemyBullet[0], SRCAND);
				putimage(enemyBullets[i][k].x, enemyBullets[i][k].y, &enemyBullet[1], SRCPAINT);
			}
	}
}

void loadAttribute()
{
	// 显示分数
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(20, 0, _T("Arial"));
	char scoreText[20];
	_stprintf_s(scoreText, _T("Score: %d"), score);
	outtextxy(10, 10, scoreText);

	// 显示血量
	char hpText[20];
	_stprintf_s(hpText, "HP: %d", player.HP);
	outtextxy(360, 10, hpText);
}

void bulletMove(int speed);
void fillingBullet();

void playerAct(int speedRow)
{
	int speed = speedRow;
	int speedLower = speed / 2;

	if (player.y >= 0)
	{
		if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W'))
		{
			player.y -= speed;
		}
	}

	if (player.y <= Height - 50)
	{
		if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S'))
		{
			player.y += speed;
		}
	}

	if (player.x >= 0)
	{
		if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))
		{
			player.x -= speed;
		}
	}

	if (player.x <= Width - 33)
	{
		if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))
		{
			player.x += speed;
		}
	}


	//还原shift降速
	if (GetAsyncKeyState(VK_LSHIFT))
	{
		if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W'))
		{
			player.y += speedLower;
		}
		if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S'))
		{
			player.y -= speedLower;
		}
		if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))
		{
			player.x += speedLower;
		}
		if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))
		{
			player.x -= speedLower;
		}
	}

	//发射弹幕
	static DWORD timef1 = 0, times1 = 0;//设置发射间隔
	if (GetAsyncKeyState(VK_SPACE) && times1 - timef1 >= 80)
	{
		fillingBullet();
		timef1 = times1;
	}
	times1 = GetTickCount();

	//防止血量低于下限
	if (!player.exist)
	{
		player.HP = 0;
	}
}

void fillingBullet()
{
	for (int i = 0; i < BulletNum; i++)
	{
		if (!bullet[i].exist)
		{
			bullet[i].exist = true;
			bullet[i].x = player.x + 11;
			bullet[i].y = player.y - 30;
			break;
		}
	}
}

void bulletMove(int speed)
{
	for (int i = 0; i < BulletNum; i++)
	{
		if (bullet[i].exist)
		{
			bullet[i].y -= speed * 2;
			if (bullet[i].y <= -42)
			{
				bullet[i].exist = false;
			}
			else
			{
				for (int j = 0; j < EnemyNum; j++)
				{
					if (enemy[j].exist)
					{
						if (bullet[i].x >= enemy[j].x && bullet[i].x <= enemy[j].x + enemy[j].width &&
							bullet[i].y >= enemy[j].y && bullet[i].y <= enemy[j].y + enemy[j].height)
						{
							enemy[j].HP -= 20;
							bullet[i].exist = false;
							if (enemy[j].HP <= 0)
							{
								//记录分数
								enemy[j].exist = false;
								if (enemy[j].type == Large) {
									score += 500;
								}
								else {
									score += 200;
								}
							}
							break;
						}
					}
				}
			}
		}
	}
}


void enemyAttribute(int j)
{
	if (rand() % 4 == 0)
	{
		enemy[j].type = Large;
		enemy[j].HP = 80;
		enemy[j].width = 60;
		enemy[j].height = 60;
	}
	else {
		enemy[j].type = Small;
		enemy[j].HP = 20;
		enemy[j].width = 30;
		enemy[j].height = 30;
		if (rand() % 2 == 0)
		{
			enemy[j].type = SmallN;
		}
		else {
			enemy[j].type = SmallM;
		}
	}
}

void creatingEnemy() {
	for (int i = 0; i < EnemyNum; i++)
	{
		if (!enemy[i].exist)
		{
			enemy[i].exist = true;
			enemyAttribute(i);
			if (enemy[i].type == Large)
			{
				enemy[i].x = rand() % (Width - 63);
				enemy[i].y = -63;
			}
			else if (enemy[i].type == SmallN)
			{
				if (rand() % 2 == 0)
				{
					enemy[i].type = SmallNL;
					enemy[i].x = -30;  // 从左边出现
				}
				else {
					enemy[i].type = SmallNR;
					enemy[i].x = Width;  // 从右边出现
				}
				enemy[i].y = 120;
			}
			else {
				if (rand() % 2 == 0)
				{
					enemy[i].type = SmallML;
					enemy[i].x = -30;  // 从左边出现
				}
				else
				{
					enemy[i].type = SmallMR;
					enemy[i].x = Width;  // 从右边出现
				}
				enemy[i].y = -30;
			}
			break;

		}
	}
}


void enemyMove(int speed)
{
	for (int i = 0; i < EnemyNum; i++)
	{
		if (enemy[i].type == Large)
		{
			enemy[i].y += speed;
		}
		else if (enemy[i].type == SmallNL || enemy[i].type == SmallNR)
		{
			if (enemy[i].type == SmallNL)
			{
				enemy[i].x += speed;
				enemy[i].y = (-enemy[i].x * enemy[i].x + 330 * enemy[i].x + 420 * 90) / 500;
			}
			else {
				enemy[i].x -= speed;
				enemy[i].y = (-enemy[i].x * enemy[i].x + 330 * enemy[i].x + 420 * 90) / 500;
			}
		}
		else {
			if (enemy[i].type == SmallML)
			{
				enemy[i].x += speed;
				enemy[i].y = enemy[i].x / 2;
			}
			else {
				enemy[i].x -= speed;
				enemy[i].y = enemy[i].x / 2;
			}
		}

		//碰撞
		if (enemy[i].exist)
		{
			if (enemy[i].x + enemy[i].width >= player.x + 6 && enemy[i].x <= player.x + 14 &&
				enemy[i].y >= player.y + 18 && enemy[i].y <= player.y + 26)
			{
				enemy[i].exist = false;
				if (player.HP > 0)
				{
					player.HP -= 40;
				}
			}
		}


		//设置超出限制
		if (enemy[i].x > Width || enemy[i].x < -40)
		{
			enemy[i].exist = false;
		}
	}
}

void enemyBulletMove(int speed)
{
	for (int i = 0; i < EnemyNum; i++)
	{
		if (enemy[i].exist)
		{
			if (rand() % 50 == 0)
			{
				for (int k = 0; k < 5; k++)
				{
					if (!enemyBullets[i][k].exist)
					{
						enemyBullets[i][k].exist = true;
						enemyBullets[i][k].x = enemy[i].x + enemy[i].width / 2;
						enemyBullets[i][k].y = enemy[i].y + enemy[i].height;
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < EnemyNum; i++)
	{
		for (int k = 0; k < 5; k++)
		{
			if (enemyBullets[i][k].exist)
			{
				enemyBullets[i][k].y += speed;
				if (enemyBullets[i][k].y > Height)
				{
					enemyBullets[i][k].exist = false;
				}
				if (enemyBullets[i][k].x >= player.x + 6 && enemyBullets[i][k].x <= player.x + 14 &&
					enemyBullets[i][k].y >= player.y + 18 && enemyBullets[i][k].y <= player.y + 26)
				{
					enemyBullets[i][k].exist = false;
					if (player.HP > 0)
					{
						player.HP -= 20;
					}
				}
			}
		}
	}
}

void readScores() {
	FILE* fp = fopen("scores.txt", "r");

	for (int i = 0; i < MAX_SCORES; i++)
	{
		if (fscanf(fp, "%d-%ld\n", &topScores[i].score, &topScores[i].timestamp) != 2)
		{
			topScores[i].score = 0;
			topScores[i].timestamp = 0;
		}
	}
	fclose(fp);
}

void writeScores() {
	FILE* fp = fopen("scores.txt", "w");
	for (int i = 0; i < MAX_SCORES; i++)
	{
		fprintf(fp, "%d-%ld\n", topScores[i].score, topScores[i].timestamp);
	}

	fclose(fp);
}


void updateScores(int newScore)
{
	readScores();
	Score newScoreEntry;
	newScoreEntry.score = newScore;
	newScoreEntry.timestamp = time(NULL);

	// 将新分数加入数组中并排序
	int insertIndex = -1;
	for (int i = 0; i < MAX_SCORES; i++)
	{
		if (newScoreEntry.score > topScores[i].score)
		{
			insertIndex = i;
			break;
		}
	}

	if (insertIndex != -1) {
		// 插入新分数
		for (int i = MAX_SCORES - 1; i > insertIndex; i--)
		{
			topScores[i] = topScores[i - 1];
		}
		topScores[insertIndex] = newScoreEntry;
		writeScores();
	}
}

// 显示排行榜
void displayScores()
{

	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	settextstyle(22, 0, _T("Arial"));
	char endTitle[50];
	char scoreText[50];
	outtextxy(40, 140, TEXT("Rank:"));
	for (int i = 0; i < MAX_SCORES; i++)
	{
		if (topScores[i].score > 0)
		{
			_stprintf_s(scoreText, _T("%d - %d - %s"), i + 1, topScores[i].score, ctime(&topScores[i].timestamp));
			outtextxy(40, 170 + i * 30, scoreText);
		}
	}
}

void Initialization()
{
	player.x = Width / 2 - 17;
	player.y = Height - 80;
	player.HP = 100;
	player.exist = true;

	//初始化分数
	score = 0;

	//初始化Bullet
	for (int i = 0; i < BulletNum; i++)
	{
		bullet[i].exist = false;
		bullet[i].x = 0;
		bullet[i].y = 0;
	}

	//初始化Elf
	for (int j = 0; j < EnemyNum; j++)
	{
		enemy[j].exist = false;
	}
}


int main()
{
	//设置窗口
	initgraph(Width, Height);

	// 初始化随机数种子
	srand(time(NULL));

	//初始化
	Initialization();

	mciSendString(_T("play bgm1.mp3"), NULL, 0, NULL);

	//双缓冲
	BeginBatchDraw();

	bool gameRunning = true;
	while (gameRunning)
	{

		loadGame();
		loadAttribute();

		FlushBatchDraw();

		if (player.exist)
		{
			playerAct(2);
		}
		bulletMove(2);

		static DWORD timef2, times2;
		if (times2 - timef2 > 400)
		{
			creatingEnemy();
			timef2 = times2;
		}
		times2 = clock();

		static DWORD timef3, times3;
		if (times3 - timef3 > 50)
		{
			enemyBulletMove(2);
			timef3 = times3;
		}
		times3 = clock();

		enemyMove(1);

		if (!player.exist) 
		{
			updateScores(score);
			displayScores();
			break;
		}
	}

	EndBatchDraw();
	while (true)
	{
		if(GetAsyncKeyState(VK_ESCAPE)) 
		{
			break;
		}
	}
	return 0;
}