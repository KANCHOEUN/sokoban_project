#include <stdio.h>
#include <termio.h>
#include <stdlib.h>

#define Y 30
#define X 30
#define STAGE 5

int map[STAGE][Y][X];
int player[STAGE][Y][X];
int player_x[STAGE], player_y[STAGE];
int x, y;
int stage = -1;
int box_cnt = 0;
int clear_cnt = 0;

int getch();
void MapLoad();
void MapDraw();
void MapClear();
void PlayerMove();

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
      box_cnt++;
    }

    if(ch == 'O')
    {
      clear_cnt++;
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

  stage = 0;

  fclose(fp);
}

void MapDraw()
{
  if(box_cnt != clear_cnt)
  {
    printf("ERROR : 박스 개수와 도착 지점의 개수가 다릅니다.\n");
    exit(1);
  }

  map[stage][player_y[stage]][player_x[stage]] = '@';

  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      printf("%c", map[stage][i][j]);
    }
    printf("\n");
  }
}

void MapClear()
{
  system("clear");
  system("clear");
}

void PlayerMove()
{
  char key;

  key = getch();
  map[stage][player_y[stage]][player_x[stage]] = ' ';

  switch(key)
  {
    case 'h':
    case 'H':
      player_x[stage]--;
      break;

    case 'j':
    case 'J':
      player_y[stage]++;
      break;

    case 'k':
    case 'K':
      player_y[stage]--;
      break;

    case 'l':
    case 'L':
      player_x[stage]++;
      break;

    case 'e':
    case 'E':
      printf("프로그램을 종료합니다.");
      exit(0);
      break;
  }
}

int main()
{
  MapLoad();

  MapDraw();
  while(1)
  {
    MapClear();
    MapDraw();
    PlayerMove();
  }

  return 0;
}
