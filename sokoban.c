#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

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
char username[11];
int t_bl = 0;

int getch();
void MapLoad();
void MapDraw();
void MapClear();
void PlayerMove();
void UserName();
void FileLoad();
bool StageClear();
void Option(char key);

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
  StageClear();

  printf("    Hello %s\n\n", username);

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

void Save()
{
  FILE *save;
  int i;

  save = fopen("sokoban.txt", "w");

  fprintf(save, "%s\n", username);

  fprintf(save, "%d\n", stage);

  i = stage;

  for(int j = 0; j < Y; j++)
  {
    for(int k = 0; k < X; k++)
    {
      fprintf(save, "%c", map[i][j][k]);
    }
    fprintf(save, "\n");
  }

  fclose(save);
}

void FileLoad()
{
  FILE *fileload;
  char ch;
  int i = 0;
  int j;
  int load_map[Y][X];
  int load_x = 0, load_y = -2;
  int line_cnt = 0;

  if ((fileload = fopen("sokoban.txt", "r")) == NULL)
  {
    printf("\n\n\nLoad File Doesn't Exist.\n\n");

    exit(1);
  }
  fclose(fileload);

  for(int i = 0; i < 11; i++)
  {
    username[i] = ' ';
  }

  fileload = fopen("sokoban.txt", "r");

  while(fscanf(fileload, "%c", &ch) != EOF)
  {
    if(ch == '\n')
    {
      load_x = 0;
      load_y++;
      line_cnt++;
    }
    else if ((line_cnt == 0) && (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z')))
    {
      username[i] = ch;
      ++i;
    }
    else if ((line_cnt == 1) && ('0' <= ch && ch <= '9'))
    {
      stage = ch - '0';
    }
    else
    {
      load_map[load_y][load_x] = ch;
      load_x++;
    }

    if(ch == '@')
    {
      player_y[stage] = load_y;
      player_x[stage] = load_x - 1;
    }
  }

  username[++i] = '\0';

  for(int a = 0; a < Y; a++)
  {
    for(int b = 0; b < X; b++)
    {
      map[stage][a][b] = load_map[a][b];
    }
  }

  fclose(fileload);
}

void UserName()
{
  bool flag;
  int i = 0;
  int length;

  printf("Start....\n");

  for(int i = 0; i < 11; i++)
  {
    username[i] = ' ';
  }

  while(1)
  {
    flag = true;
    length = 0;
    printf("input name : ");
    scanf("%s", username);

    for(int i = 0; i < 11; i++)
    {
      if(!(('a' <= username[i] && username[i] <= 'z') || ('A' <= username[i] && username[i] <= 'Z')) && (username[i] != '\0'))
      {
        flag = false;
      }

      if(username[i] == '\0')
        break;
    }

    while(username[length] != '\0')
    {
      if(length > 9)
        flag = false;

      length++;
    }

    if(flag)
      break;

    printf("Name Must Be In English And Do Not Exceed 10 Letters.\n");
  }

  getch();
  sleep(1);
  MapClear();
}

void PlayerMove()
{
  char key;
  int dx = 0, dy = 0;

  printf("(Command) ");

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

    case '*':
      stage++;
      break;
  }

  if(((((((key != 'h' && key != 'j') && key != 'k') && key != 'l') && key != 'H') && key != 'J') && key != 'K') && key != 'L')
  {
    Option(key);
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
    printf("\n\nCongratulations !\n");
    printf("\nAll Stage Clear !\n\n");
    exit(0);
  }
  return flag;
}

void Option(char key)
{
  char enter;
  int Top_i;

  if(((((((key != 'h' && key != 'j') && key != 'k') && key != 'l') && key != 'H') && key != 'J') && key != 'K') && key != 'L')
    printf("%c", key);

  enter = getch();

  switch (key)
  {
      case 's':
      case 'S':
      if(enter == '\n')
        Save();
        break;

      case 'f':
      case 'F':
      if(enter == '\n')
        FileLoad();
        break;

      case 't':
      case 'T':
        if(t_bl == 0)
        {
          t_bl = 1;
        }
        switch (enter)
        {
          case '1' :
            Top_i = 1;
            printf("1\n");
            break;

          case '2' :
            Top_i = 2;
            printf("2\n");

          case '3' :
            Top_i = 3;
            printf("3\n");

          case '4' :
            Top_i = 4;
            printf("4\n");

          case '5' :
            Top_i = 5;
            printf("5\n");

          case '\n' :
            Top_i = -1;
            break;

          default :
            Top_i = -1;
            printf("\n-----------------------------------\n\n       Command Doesn't Exit.\n\n-----------------------------------\n");
            sleep(1);
            break;
        }

        while(1)
        {
          if(Top_i == 0)
          {

            break;
          }
          if(Top_i == -1)
          {
            break;
          }
          if(getch() == '\n' && Top_i != 0 && Top_i != -1)
          {

            break;
          }
        }
        break;

      case 'e':
      case 'E':
      if(enter == '\n')
        MapClear();
        printf("\n\n\nSEE YOU %s . . . .\n\n\n", username);
        Save();
        exit(0);
        break;

      default :
      if(enter == '\n')
        printf("\n-----------------------------------\n\n       Command Doesn't Exist.\n\n-----------------------------------\n");
        sleep(1);
        break;
  }
}

int main()
{
  UserName();
  MapLoad();

  while(1)
  {
    MapClear();
    MapDraw();
    PlayerMove();
  }

  return 0;
}
