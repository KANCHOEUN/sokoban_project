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
int d_bl = 0, t_bl = 0;
//d_bl은 벌룬값 0은 display들어가기전 1은 나가기전
char score_name[STAGE][5][200];
int score_time[STAGE][5];

int getch();
void MapLoad();
void MapDraw();
void MapClear();
void Save();
void PlayerMove();
void Display();
void Replay();
void New();
void Top();

int getch(void)
{

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

  if(box_cnt != clear_cnt)
  {
    printf("ERROR : 박스 개수와 도착 지점의 개수가 다릅니다.\n");
    exit(1);
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
      Save();
       exit(0);
      break;

    case 'd' :
    case 'D' :
      if(d_bl==0)
      {
        d_bl = 1;
        Display();
      }
      break;

    case 'r' :
    case 'R' :
      Replay();
      break;

    case 'n' :
    case 'N' :
      New();
      break;

    case 't' :
    case 'T' :
      if(t_bl==0)
      {
        t_bl = 1;
        Top();
      }
      break;

    //치트키
    case '!' :
      stage += 1;
      break;
    //맵넘어가기

    //치트키
  }

  map[stage][player_y[stage]][player_x[stage]] = ' ';

  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == '#')
  {
    dx = 0;
    dy = 0;
  }

  player_x[stage] += dx;
  player_y[stage] += dy;
  map[stage][player_y[stage]][player_x[stage]] = '@';

}

void Display()
{
  while(1)
  {
    MapClear();
    printf("h : 왼쪽으로 이동, j : 아래로 이동, k : 위로 이동, l : 오른쪽으로 이동");
    printf("\n");
    printf("u : 움직이기 전 상태로 이동한다. (최대 5번 가능)");
    printf("\n");
    printf("r : 현재 앱을 처음부터 다시시작한다.");
    printf("\n");
    printf("n : 첫 번째 맵부터 다시 시작");
    printf("\n");
    printf("e : 게임종료");
    printf("\n");
    printf("s : 게임 저장");
    printf("\n");
    printf("f : 게임을 이어서한다.");
    printf("\n");
    printf("d : 명령내용을 보여준다.");
    printf("\n");
    printf("t : 게임 순위를 보여준다.");
    printf("\n");
    printf("나가려면 d키를 누르시오.");
    if(getch()=='d' & d_bl == 1)
    {
      d_bl = 0;
      break;
    }
  }
  return;
}

void Replay()
{
  int stage_tmp = stage;
  MapClear();
  stage = -1;
  MapLoad();
  stage = stage_tmp;

}

void New()
{
  MapClear();
  stage = -1;
  MapLoad();
}

void Top()
{
  FILE *fp;
  char username[200];
  int score;

  fp = fopen("ranking.txt", "r");

  while((fscanf(fp, "%s %d", &username, &score)) != EOF)
  {
    for(int i = 0; i < STAGE; i++)
    {
      for(int j = 0; j < 5; j++)
      {
        for(int k=0; k<200; k++)
        {
          score_name[i][j][k] = username[k];
          if(username[k] == '\0')
          {
            break;
          }
        }
        score_time[i][j] = score;
        fscanf(fp, "%s %d", &username, &score);
      }
    }
  }
  while(1)
  {
    MapClear();
    for(int i = 0; i < STAGE; i++)
    {
      for(int j = 0; j < 5; j++)
      {
        printf("%s  ", score_name[i][j]);
        printf("%d\n", score_time[i][j]);
      }
    }
    printf("나가려면 t키를 누르시오.");
    if(getch()=='t' & t_bl == 1)
    {
      t_bl = 0;
      break;
    }
  }
  return;

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
