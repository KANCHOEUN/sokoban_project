#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define Y 30
#define X 30
#define STAGE 5

int map[STAGE][Y][X];
int origin_map[STAGE][Y][X];
int player[STAGE][Y][X];
int undo[STAGE][Y][X];
int player_x[STAGE], player_y[STAGE];
int origin_player_x[STAGE], origin_player_y[STAGE];
int x, y;
int stage = -1;
int box_cnt[STAGE];
int clear_cnt[STAGE];
int save_count = 0;
int undo_count = 5;
int move_count = 0;
char username[11];
int d_bl = 0, t_bl = 0;
char score_name[STAGE][5][11];
double score_time[STAGE][5];
clock_t start_clock, end_clock, stop_clock;
double diff_clock, sum_stop = 0;
double score;

int getch();
void MapLoad();
void MapDraw();
void MapClear();
void PlayerMove();
void UndoMap();
void Undo();
void UserName();
void FileLoad();
bool StageClear();
void Option(char key);
void Display();
void Replay();
void New();
void StartTime();
void EndTime();
void Top(int Top_i);
void SaveTop();
void StopTime();

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

      origin_player_x[stage] = x;
      origin_player_y[stage] = y;
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

  fprintf(save, "%d\n", undo_count);

  fprintf(save, "%d\n", move_count);

  fprintf(save, "%d\n", save_count);

  fprintf(save, "%.1f\n", score);

  i = stage;

  for(int j = 0; j < Y; j++)
  {
    for(int k = 0; k < X; k++)
    {
      fprintf(save, "%c", map[i][j][k]);
    }
    fprintf(save, "\n");
  }


  for(int a = 0; a < 5; a++)
  {
    fprintf(save, "U\n");
    for(int b = 0; b < Y; b++)
    {
      for(int c = 0; c < X; c++)
      {
        fprintf(save, "%c", undo[a][b][c]);
      }
      fprintf(save, "\n");
    }
  }
  fclose(save);
}

void FileLoad()
{
  FILE *fileload;
  char ch;
  int load_map[Y][X];
  int load_x = 0, load_y = 0, load_z;

  fileload = fopen("sokoban.txt", "r");
  if (fileload == NULL)
  {
    printf("\n\n\nLoad File Doesn't Exist.\n\n");

    exit(1);
  }
  //fclose(fileload);

  // for(int i = 0; i < 11; i++)
  // {
  //   username[i] = ' ';
  // }
  fscanf(fileload,"%s\n", username);
  fscanf(fileload,"%d\n%d\n%d\n%d\n%f", &stage, &undo_count, &move_count, &save_count, &score);
  fscanf(fileload, "%c", &ch);
  int line = 0;
  while(fscanf(fileload, "%c", &ch) != EOF)
  {
    if(ch == '\n')
    {
      line++;
      load_x = 0;
      load_y++;
    }
    else if(line == 29)
    {
      load_x = 0;
      load_y = 0;
      load_z = -1;
      break;
    }
    else
    {
    map[stage][load_y][load_x] = ch;
    load_x++;
    }

    if(ch == '@')
    {
      player_x[stage] = load_x - 1;
      player_y[stage] = load_y;
    }
  }
  while(fscanf(fileload, "%c", &ch) != EOF)
  {
    if(ch == 'U')
    {
      load_x = 0;
      load_y = 0;
      load_z++;
      fscanf(fileload,"%c", &ch);
    }
    else if(ch == '\n')
    {
      load_x = 0;
      load_y++;
    }
    else
    {
      undo[load_z][load_y][load_x] = ch;
      load_x++;
    }
  }
  // username[++i] = '\0';

  printf("\n");
  for(int a = 0; a < 5; a++)
  {
    //printf("U\n");
    for(int b = 0; b < Y; b++)
    {
      for(int c = 0; c < X; c++)
      {
        printf("%c", undo[a][b][c]);
      }
      printf("\n");
    }
  }
  // sleep(10);
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
      UndoMap();
      map[stage][player_y[stage] + dy][player_x[stage] + dx] = '@';
      map[stage][player_y[stage] + 2*dy][player_x[stage]+ 2*dx] = '$';
    }
    else if(map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '#' || map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '$')
    {
      dx = 0;
      dy = 0;
    }
  }

  if(map[stage][player_y[stage]+ dy][player_x[stage] + dx] == ' ')
  {
    UndoMap();
  }

  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == 'O')
  {
    UndoMap();
  }

  if(map[stage][player_y[stage] + 2 * dy][player_x[stage] + 2 * dx] == 'O' &&map[stage][player_y[stage] + dy][player_x[stage] + dx] == '$')
  {
    UndoMap();
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

  if(!(dx == 0 && dy == 0))
  {
    move_count++;
  }

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
    SaveTop();
    stage++;
    flag = true;
    score = 0;
    sum_stop = 0;
    StartTime();
  }

  if(stage >= 5)
  {
    printf("\n\nCongratulations !\n");
    printf("\nAll Stage Clear !\n\n");
    exit(0);
  }

  if(flag)
  {
    for(int k = 0; k < STAGE; k++)
    {
      for(int i = 0; i < Y; i++)
      {
        for(int j = 0; j < X; j++)
        {
          undo[k][i][j] = ' ';
        }
      }
    }

    save_count = 0;
    undo_count = 5;
    move_count = 0;
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

      case 'd' :
      case 'D' :
        if(d_bl==0)
        {
          d_bl = 1;
          stop_clock = clock();
          Display();
        }
        StopTime();
        break;

      case 'r' :
      case 'R' :
        Replay();
        break;

      case 'n' :
      case 'N' :
        New();
        break;


      case 't':
      case 'T':
        if(t_bl == 0)
        {
          t_bl = 1;
          stop_clock = clock();
        }
        switch (enter)
        {
          case '1' :
            printf("1");
            Top_i = 1;
            break;

          case '2' :
            printf("2");
            Top_i = 2;
            break;

          case '3' :
            printf("3");
            Top_i = 3;
            break;

          case '4' :
            printf("4");
            Top_i = 4;
            break;

          case '5' :
            printf("5");
            Top_i = 5;
            break;

          case '\n' :
            Top_i = 0;
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
            Top(0);
            break;
          }
          if(Top_i == -1)
          {
            break;
          }
          if(getch() == '\n' && Top_i != 0 && Top_i != -1)
          {
            Top(Top_i);
            break;
          }
        }
        StopTime();
        break;

      case 'e':
      case 'E':
      if(enter == '\n')
        MapClear();
        printf("\n\n\nSEE YOU %s . . . .\n\n\n", username);
        Save();
        exit(0);
        break;

      case 'u':
      case 'U':
      if(enter == '\n')
        Undo();
        break;

      default :
      if(enter == '\n')
        printf("\n-----------------------------------\n\n       Command Doesn't Exist.\n\n-----------------------------------\n");
        sleep(1);
        break;
  }
}

void Display()
{
  while(1)
  {
    MapClear();
    printf("h : 왼쪽으로 이동, j : 아래로 이동, k : 위로 이동, l : 오른쪽으로 이동\n");
    printf("u : 움직이기 전 상태로 이동한다. (최대 5번 가능)\n");
    printf("r : 현재 맵을 처음부터 다시시작한다.\n");
    printf("n : 첫 번째 맵부터 다시 시작\n");
    printf("e : 게임종료\n");
    printf("s : 게임 저장\n");
    printf("f : 게임을 이어서한다.\n");
    printf("d : 명령내용을 보여준다.\n");
    printf("t : 게임 순위를 보여준다.\n");
    printf("나가려면 d키를 누르시오.\n");
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
  int i, j, k;

  for(int k = 0; k < STAGE; k++)
  {
   for(int i = 0; i < Y; i++)
    {
      for(int j = 0; j < X; j++)
      {
        undo[k][i][j] = ' ';
      }
    }
  }

  move_count = 0;
  undo_count = 5;
  save_count = 0;

  i = stage_tmp;
  for(j = 0; j < Y; j++)
  {
    for(k = 0; k < X; k++)
    {
      map[i][j][k] = origin_map[i][j][k];
    }
  }
  player_x[i] = origin_player_x[i];
  player_y[i] = origin_player_y[i];
}

void New()
{
  StartTime();
  MapClear();
  stage = 0;
  sum_stop = 0;

  int i, j, k;

  for(int k = 0; k < STAGE; k++)
  {
   for(int i = 0; i < Y; i++)
    {
      for(int j = 0; j < X; j++)
      {
        undo[k][i][j] = ' ';
      }
    }
  }

  move_count = 0;
  undo_count = 5;
  save_count = 0;

  for(i = 0; i < STAGE; i++)
  {
    for(j = 0; j < Y; j++)
    {
      for(k = 0; k < X; k++)
      {
        map[i][j][k] = origin_map[i][j][k];
      }
    }
    player_x[i] = origin_player_x[i];
    player_y[i] = origin_player_y[i];
  }
}

void SaveTop()
{
  FILE *fp;
  char name[11];
  char a;
  double record;
  int i, j, k;

  fp = fopen("ranking.txt", "r");

  //입력
  while(1)
  {
    for(i = 0; i < STAGE; i++)
    {
      for(j = 0; j < 5; j++)
      {
        fscanf(fp, "%s %lf\n", &name, &record);
        for(k=0; k<11; k++)
        {
          if(name[0] == '0' && name[1] == '.' && name[2] == '0' && name[3] == '\0')
          {
            score_name[i][j][0] = ' ';
            score_name[i][j][1] = ' ';
            score_name[i][j][2] = '\0';
            break;
          }
          if(name[k] == '\0')
          {
            break;
          }
          score_name[i][j][k] = name[k];
        }
        score_time[i][j] = record;
      }
    }
    if((fscanf(fp, "%c", &a)) == EOF)
    {
      break;
    }
  }

  //배열
  for(i = 0; i < 4; i++)
  {
    if(score_time[stage][i] > score || score_time[stage][i] == 0 )
    {
      for(j = 0; j < 4-i; j++)
      {
        score_time[stage][4-j] = score_time[stage][3-j];
        for(k = 0; k < 11; k++)
        {
          score_name[stage][4-j][k] = score_name[stage][3-j][k];
        }
      }
      for(k = 0; k < 11; k++)
      {
        score_name[stage][i][k] = username[k];
      }
      score_time[stage][i] = score;
      break;
    }
  }


  //가장 클때
  if(score_time[stage][i] > score || score_time[stage][i] == 0)
  {
    score_time[stage][i] = score;

    for(k = 0; k < 11; k++)
    {
      score_name[stage][i][k] = username[k];
    }
  }

  fclose(fp);

//저장
  fp = fopen("ranking.txt", "w");

  for(i = 0; i < 5; i++)
  {
    for(j = 0; j < 5; j++)
    {
      if(score_name[i][j][0] == ' ' && score_name[i][j][1] == ' ' && score_name[i][j][2] == '\0')
      {
        fprintf(fp, "0.0 ", score_name[i][j]);
      }
      else
      {
        fprintf(fp, "%s ", score_name[i][j]);
      }
      fprintf(fp, "%.1f\n", score_time[i][j]);
    }
  }

  fclose(fp);

}

void Top(int Top_num)
{
  FILE *fp;
  char name[11];
  char a;
  double record;
  int i, j, k;

  fp = fopen("ranking.txt", "r");

  //입력
  while(1)
  {
    for(i = 0; i < STAGE; i++)
    {
      for(j = 0; j < 5; j++)
      {
        fscanf(fp, "%s %lf\n", &name, &record);
        for(k=0; k<11; k++)
        {
          if(name[0] == '0' && name[1] == '.' && name[2] == '0')
          {
            score_name[i][j][0] = ' ';
            score_name[i][j][1] = ' ';
            score_name[i][j][2] = '\0';
            break;
          }
          if(name[k] == '\0')
          {
            break;
          }
          score_name[i][j][k] = name[k];
        }
        score_time[i][j] = record;
      }
    }
    if((fscanf(fp, "%c", &a)) == EOF)
    {
      break;
    }
  }

  int s = 0;

  //출력
  while(1)
  {
    MapClear();
    if(Top_num == 0)
    {
      for(i = 0; i < STAGE; i++)
      {
        printf("map %d\n\n", i+1);
        for(j = 0; j < 5; j++)
        {
          for(k = 0; k < 11; k++)
          {
            if(score_name[i][j][0] == ' ' && score_name[i][j][1] == ' ' && score_name[i][j][2] == '\0')
            {
              s = 0;
              break;
            }
            else
            {
              s = 1;
              break;
            }
          }
          if(s == 1)
          {
            printf("%s  ", score_name[i][j]);
            printf("%.1f sec", score_time[i][j]);
          }
          printf("\n");
        }
      }
    }
    if(Top_num != 0)
    {
      i = Top_num-1;
      printf("map %d\n\n", i+1);
      for(j = 0; j < 5; j++)
      {
        for(k = 0; k < 11; k++)
        {
          if(score_name[i][j][0] == ' ' && score_name[i][j][1] == ' ' && score_name[i][j][2] == '\0')
          {
            s = 0;
            break;
          }
          else
          {
            s = 1;
            break;
          }
        }
        if(s == 1)
        {
          printf("%s  ", score_name[i][j]);
          printf("%.1f sec", score_time[i][j]);
        }
        printf("\n");
      }
    }
    printf("나가려면 t키를 누르시오.");
    if(getch()=='t' && t_bl == 1)
    {
      t_bl = 0;
      break;
    }
  }
  return;

}

void StartTime()
{
  start_clock = clock();
}

void EndTime()
{
  end_clock = clock();

  diff_clock = (double)(end_clock - start_clock) / 1000;
  score = diff_clock - sum_stop;
}

void StopTime()
{
  end_clock = clock();

  sum_stop += (double)(end_clock - stop_clock) / 1000;
}

void UndoMap()
{
  int i, j, k = 0;

  if(save_count >= 5)
  {
    for(i = 1; i < STAGE; i++)
    {
      for(j = 0; j < Y; j++)
      {
        for(k = 0; k < X; k++)
        {
          undo[i - 1][j][k] = undo[i][j][k];
        }
      }
    }
    save_count--;
  }

  for(j = 0; j < Y; j++)
  {
    for(k = 0; k < X; k++)
    {
      undo[save_count][j][k] = map[stage][j][k];
    }
  }
  save_count++;
}

void Undo()
{
  int undo_x = 0, undo_y = 0;

  if((undo_count < 1) || ((5 - undo_count) >= move_count))
    return;

  undo_count--;
  save_count--;

  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      map[stage][i][j] = undo[save_count][i][j];
    }
  }

  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      if(map[stage][i][j] == '@')
      {
        undo_x = j;
        undo_y = i;
      }
    }
  }

  player_y[stage] = undo_y;
  player_x[stage] = undo_x;
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
    EndTime();
  }

  return 0;
}
