#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <stdbool.h>

#define Y 30
#define X 30
#define STAGE 5

int map[STAGE][Y][X];
int origin_map[STAGE][Y][X];
int player[STAGE][Y][X];
int player_x[STAGE], player_y[STAGE];
int x, y;
int stage = -1;
int box_cnt[STAGE];
int clear_cnt[STAGE];

int getch();
void MapLoad();
void MapDraw();
void MapClear();
void PlayerMove();
bool StageClear();

int getch(void){

   int ch;
   struct termios buf;
   struct termios save;

   tcgetattr(0, &save);

   buf = save;
   buf.c_lflag&=~(ICANON|ECHO);
   buf.c_cc[VMIN] = 1;
   buf.c_cc[VTIME] = 0;

   tcsetattr(0, TCSAFLUSH, &buf);

   ch = getchar();

   tcsetattr(0, TCSAFLUSH, &save);

   return ch;

}

void MapLoad()
{
  FILE *fp;
  char ch;
  int x = 0, y = 0;

  fp = fopen("map.txt", "r");

  while((fscanf(fp, "%c", &ch)) != EOF)
  {
    if(ch == 'm')
    {
      stage++;
      y = -1;
      continue;
    }
    else if(ch == 'a' || ch == 'p')
    {
      continue;
    }

    if(ch == 'e')
    {
      break;
    }

    if(ch == '@')
    {
      player_x[stage] = x;
      player_y[stage] = y;
    }

    if(ch == '$')
    {
      box_cnt[stage]++;
    }

    if(ch == 'O')
    {
      clear_cnt[stage]++;
    }

    if(ch == '\n')
    {
      y++;
      x = 0;
    }
    else
    {
      map[stage][y][x] = ch;

      x++;
    }
  }

  for(int i = 0; i < STAGE; i++)
  {
    for(int j = 0; j < Y; j++)
    {
      for(int k = 0; k < X; k++)
      {
        origin_map[i][j][k] = map[i][j][k];
      }
    }
  }

  for(int i = 0; i < STAGE; i++)
  {
    if(box_cnt[i] != clear_cnt[i])
    {
      printf("ERROR : 박스 개수와 도착 지점의 개수가 다릅니다.\n");
      exit(1);
    }
  }

  stage = 0;

  fclose(fp);
}

void MapDraw()
{
  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      printf("%c", map[stage][i][j]);
    }
    printf("\n");
  }
  StageClear();
}

void MapClear()
{
  system("clear");
  system("clear");
}

void PlayerMove()
{
  char key;
  int dx = 0, dy = 0;

  key = getch();

  switch(key)
  {
    case 'h':
    case 'H':
      dx = -1;
      break;

    case 'j':
    case 'J':
      dy = 1;
      break;

    case 'k':
    case 'K':
      dy = -1;
      break;

    case 'l':
    case 'L':
      dx = 1;
      break;

    case 'e':
    case 'E':
      printf("프로그램을 종료합니다.");
      exit(0);
      break;

    case '*':
      stage++;
      break;
  }

  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == '#')
  {
    dx = 0;
    dy = 0;
  }
  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == '$')
  {
    if(map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == ' ')
    {
      map[stage][player_y[stage] + dy][player_x[stage] + dx] = '@';
      map[stage][player_y[stage] + 2*dy][player_x[stage]+ 2*dx] = '$';
    }
    else if(map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '#' || map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '$')
    {
      dx = 0;
      dy = 0;
    }
  }
  if(map[stage][player_y[stage] + 2 * dy][player_x[stage] + 2 * dx] == 'O' &&map[stage][player_y[stage] + dy][player_x[stage] + dx] == '$')
  {
    map[stage][player_y[stage] + 2 * dy][player_x[stage] + 2 * dx] = '$';
  }

  map[stage][player_y[stage]][player_x[stage]] = ' ';

  if(origin_map[stage][player_y[stage]][player_x[stage]] == 'O')
  {
    map[stage][player_y[stage]][player_x[stage]] = 'O';
  }
  player_x[stage] += dx;
  player_y[stage] += dy;
  map[stage][player_y[stage]][player_x[stage]] = '@';
}

bool StageClear()
{
  int count = 0;
  bool flag = false;

  for(int y = 0; y < Y; y++)
  {
    for(int x = 0; x < X; x++)
    {
      if(origin_map[stage][y][x] == 'O' && map[stage][y][x] == '$')
      {
        count++;
      }
    }
  }

  if(count == clear_cnt[stage])
  {
    stage++;
    flag = true;
  }
  if(stage >= 5)
  {
    printf("축하합니다\n");
    printf("당신은 모든 맵을 클리어했습니다.");
    exit(0);
  }
  return flag;
}

int main()
{
  MapLoad();

  while(1)
  {
    MapClear();
    MapDraw();
    PlayerMove();
  }

  return 0;
}
