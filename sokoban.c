#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define Y 30 // 맵 최대 Y 값
#define X 30 // 맵 최대 X 값
#define STAGE 5 // 최대 스테이지 값

int map[STAGE][Y][X]; // 맵 저장 배열
int origin_map[STAGE][Y][X]; //최초 맵 배열
int undo[STAGE][Y][X]; // 언두 맵 저장 배열
int player_x[STAGE], player_y[STAGE]; // 플레이어의 위치
int origin_player_x[STAGE], origin_player_y[STAGE]; //최초의 맵 플레이어 위치
int stage = -1; // 스테이지 초기 값
int box_cnt[STAGE]; // $ 개수
int clear_cnt[STAGE]; // O 개수
int save_count = 0; // 언두 맵 저장 횟수
int undo_count = 5; // 언두 가능 횟수
int move_count = 0; // 움직인 횟수
char username[11]; // 플레이어 이름
int d_bl = 0, t_bl = 0; // 디스플레이, 탑 옵션 실행여부
char score_name[STAGE][5][11]; // 랭킹 이름 배열
double score_time[STAGE][5]; // 랭킹 점수 배열
clock_t start_clock, end_clock, stop_clock; // 시작, 끝, 멈춘 시간
double diff_clock, sum_stop = 0; // 시간 차이, 총 멈춘 시간
double score; // 점수

int getch(); // getch함수
void MapLoad(); // 맵 로드
void MapDraw(); // 맵 그리기
void MapClear(); // 맵 클리어
void PlayerMove(); // 플레이어 이동
void UndoMap(); // 언두 맵 저장
void Undo();// 언두 기능
void UserName(); // 플레이어 이름 입력
void Save(); // 파일 저장하기
void FileLoad(); // 파일 불러오기
bool StageClear(); // 스테이지 클리어
void Option(char key); // 옵션 기능
void Display(); // 옵션 설명
void Replay(); // 현재맵을 처음부터 다시시작
void New(); // 첫번째 맵부터 다시시작
void StartTime(); // 시작시각 설정
void EndTime(); //끝난시각 설정
void Top(int Top_i); // 랭킹 보여주기
void SaveTop(); // 랭킹 기록
void StopTime(); // 멈춘 시간 측정

// getch 함수
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

// 맵 로드
void MapLoad()
{
  FILE *fp;
  char ch;
  int x = 0, y = 0;

  fp = fopen("map.txt", "r");

  while((fscanf(fp, "%c", &ch)) != EOF)
  {
    // m 일때 새로운 스테이지
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

    // e 일때 종료
    if(ch == 'e')
    {
      break;
    }

    // 플레이어 위치 기록
    if(ch == '@')
    {
      player_x[stage] = x;
      player_y[stage] = y;

      origin_player_x[stage] = x;
      origin_player_y[stage] = y;
    }

    // 상자 개수 카운트
    if(ch == '$')
    {
      box_cnt[stage]++;
    }

    // O 개수 카운트
    if(ch == 'O')
    {
      clear_cnt[stage]++;
    }

    if(ch == '\n')
    {
      y++;
      x = 0;
    }
    // 맵 저장
    else
    {
      map[stage][y][x] = ch;

      x++;
    }
  }
  // 초기 맵 저장
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

  // $개수와 O개수가 다르면 종료
  for(int i = 0; i < STAGE; i++)
  {
    if(box_cnt[i] != clear_cnt[i])
    {
      printf("ERROR : 박스 개수와 도착 지점의 개수가 다릅니다.\n");
      exit(1);
    }
  }

  // 스테이지 초기 값
  stage = 0;

  fclose(fp);
}

// 맵 그리기
void MapDraw()
{
  StageClear();

  printf("    Hello %s\n\n", username);

  // 맵 출력
  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      printf("%c", map[stage][i][j]);
    }
    printf("\n");
  }
}

// 맵 클리어
void MapClear()
{
  system("clear");
  system("clear");
}

void Save()
{
  FILE *save;
  int i;

  // sokoban.txt를 w 모드로 열기
  save = fopen("sokoban.txt", "w");

  // username, stage, undo_count, move_count, save_count, score 저장
  fprintf(save, "%s\n", username);
  fprintf(save, "%d\n", stage);
  fprintf(save, "%d\n", undo_count);
  fprintf(save, "%d\n", move_count);
  fprintf(save, "%d\n", save_count);
  fprintf(save, "%.1f\n", score);

  i = stage;
  // 현재 map 상태 저장
  for(int j = 0; j < Y; j++)
  {
    for(int k = 0; k < X; k++)
    {
      fprintf(save, "%c", map[i][j][k]);
    }
    fprintf(save, "\n");
  }

  // undo 1번, 2번, ..., 5번 했을 때 각각의 map 상태 저장
  for(int a = 0; a < 5; a++)
  {
    fprintf(save, "U\n"); // Undo 맵 구분을 위해 U 표시
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
  int line = 0;

  // sokoban.txt를 r 모드로 열기
  fileload = fopen("sokoban.txt", "r");
  // 로드한 파일이 빈 파일일 경우, 프로그램 종료하기
  if (fileload == NULL)
  {
    printf("\n\n\nLoad File Doesn't Exist.\n\n");

    exit(1);
  }

  fscanf(fileload,"%s\n", username);
  fscanf(fileload,"%d\n%d\n%d\n%d\n%f", &stage, &undo_count, &move_count, &save_count, &score);
  fscanf(fileload, "%c", &ch);

  // 파일의 끝부분까지 파일에 있는 내용 읽기
  while(fscanf(fileload, "%c", &ch) != EOF)
  {
    // ch가 '\n' 개행일 경우, x좌표 0으로 초기화, line과 y좌표 1씩 증가
    if(ch == '\n')
    {
      line++;
      load_x = 0;
      load_y++;
    }
    // 좌표 초기화하고 저장시킨 Undo 맵을 읽지 않기 위해 멈추기
    else if(line == 29)
    {
      load_x = 0;
      load_y = 0;
      load_z = -1;
      break;
    }
    // 읽어들인 문자를 대입하고 x좌표 1씩 증가
    else
    {
    map[stage][load_y][load_x] = ch;
    load_x++;
    }

    // 플레이어 위치 좌표
    if(ch == '@')
    {
      player_x[stage] = load_x - 1;
      player_y[stage] = load_y;
    }
  }

  // 파일의 끝부분까지 파일에 있는 내용 읽기 (Undo 맵 불러오기)
  while(fscanf(fileload, "%c", &ch) != EOF)
  {
    if(ch == 'U') // 'U'를 만날 때마다 좌표 초기화
    {
      load_x = 0;
      load_y = 0;
      load_z++;
      fscanf(fileload,"%c", &ch);
    }
    else if(ch == '\n') // ch가 '\n' 개행일 경우, x좌표 초기화, y좌표 1씩 증가
    {
      load_x = 0;
      load_y++;
    }
    else  // 읽어들인 문자를 대입하고 x좌표 1씩 증가
    {
      undo[load_z][load_y][load_x] = ch;
      load_x++;
    }
  }

  printf("\n");
  for(int a = 0; a < 5; a++)
  {
    for(int b = 0; b < Y; b++)
    {
      for(int c = 0; c < X; c++)
      {
        printf("%c", undo[a][b][c]);
      }
      printf("\n");
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

  // username[i]를 공백으로 초기화
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

    // username[i]의 값이 영문자 혹은 '\0'의 값이 아닐 경우
    for(int i = 0; i < 11; i++)
    {
      if(!(('a' <= username[i] && username[i] <= 'z') || ('A' <= username[i] && username[i] <= 'Z')) && (username[i] != '\0'))
      {
        flag = false;
      }

      if(username[i] == '\0')
        break;
    }

    // username[length]의 값이 '\0'일 때까지 반복
    while(username[length] != '\0')
    {
      if(length > 9) // username의 글자가 10문자 초과했을 경우
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

// 플레이어 이동
void PlayerMove()
{
  char key;
  int dx = 0, dy = 0;

  printf("(Command) ");

  // 키 입력
  key = getch();

  switch(key)
  {
    //  왼쪽 이동
    case 'h':
    case 'H':
      dx = -1;
      break;

    // 아래쪽 이동
    case 'j':
    case 'J':
      dy = 1;
      break;

    // 위쪽 이동
    case 'k':
    case 'K':
      dy = -1;
      break;

    // 오른쪽 이동
    case 'l':
    case 'L':
      dx = 1;
      break;
  }
  // h, j, k, l 이외의 명령이 들어올 경우 Option() 함수를 통하여 입력받음
  if(((((((key != 'h' && key != 'j') && key != 'k') && key != 'l') && key != 'H') && key != 'J') && key != 'K') && key != 'L')
  {
    Option(key);
  }
  // 충돌 체크
  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == '#') // 앞에 '#'벽이 있으면
  {
    dx = 0; // 움직이지 않음
    dy = 0;
  }
  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == '$') // '$'상자를 만난다면
  {
    if(map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == ' ') // 그 앞이 공백이라면
    {
      UndoMap(); // 언두 맵 저장
      map[stage][player_y[stage] + dy][player_x[stage] + dx] = '@'; // 움직인 후 플레이어 좌표 저장
      map[stage][player_y[stage] + 2*dy][player_x[stage]+ 2*dx] = '$'; // 움직인 후 상자 좌표 저장
    }
    else if(map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '#' || map[stage][player_y[stage] + 2*dy][player_x[stage] + 2*dx] == '$') // 플레이어가 상자를 밀고 있을때 상자 앞이 # 이거나 $ 이라면
    {
      dx = 0; // 움직이지 않음
      dy = 0;
    }
  }

  if(map[stage][player_y[stage]+ dy][player_x[stage] + dx] == ' ') // 플레이어가 움직인다면
  {
    UndoMap(); // 언두 맵 저장
  }

  if(map[stage][player_y[stage] + dy][player_x[stage] + dx] == 'O') // 'O'를 만난다면
  {
    UndoMap(); // 언두 맵 저장
  }

  if(map[stage][player_y[stage] + 2 * dy][player_x[stage] + 2 * dx] == 'O' &&map[stage][player_y[stage] + dy][player_x[stage] + dx] == '$') // 상자를 밀고 있고, 상자 앞에 'O'가 있다면
  {
    UndoMap(); // 언두 맵 저장
    map[stage][player_y[stage] + 2 * dy][player_x[stage] + 2 * dx] = '$'; // 상자를 앞으로 움직임
  }

  map[stage][player_y[stage]][player_x[stage]] = ' '; // 플레이어의 전 위치를 지워줌

  if(origin_map[stage][player_y[stage]][player_x[stage]] == 'O') // 원래 맵에서 'O'였으면
  {
    map[stage][player_y[stage]][player_x[stage]] = 'O'; // 'O' 계속 유지
  }

  player_x[stage] += dx; // 플레이어 좌표 설정
  player_y[stage] += dy; // 플레이어 좌표 설정
  map[stage][player_y[stage]][player_x[stage]] = '@'; // @에 플레이어 좌표 대입

  if(!(dx == 0 && dy == 0)) // 움직임이 있다면
  {
    move_count++; // move_count를 올린다
  }

}
// 스테이지 클리어
bool StageClear()
{
  int count = 0;
  bool flag = false;

  for(int y = 0; y < Y; y++)
  {
    for(int x = 0; x < X; x++)
    {
      if(origin_map[stage][y][x] == 'O' && map[stage][y][x] == '$') // 원래 맵의 O의 좌표와 현재 맵의 $의 좌표가 같을 경우
      {
        count++; // count값이 증가
      }
    }
  }

  if(count == clear_cnt[stage]) // count 값이 스테이지 클리어를 위한 값과 같을 때
  {
    SaveTop(); //점수 기록
    stage++; // 스테이지 증가
    flag = true; // flag에 true값
    score = 0; //점수 초기화
    sum_stop = 0; //멈춘시간 초기화
    StartTime(); //시작시간 재정의
  }

  if(stage >= 5) // 5스테이지 까지 클리어할 경우
  {
    printf("\n\nCongratulations !\n"); // 축하 문구전송
    printf("\nAll Stage Clear !\n\n");
    exit(0); // 프로그램 종료
  }

  if(flag) // 스테이지가 올라가면 언두저장맵을 초기화시킴
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

    save_count = 0; // 스테이지가 올라가면 세이브 카운트 초기화
    undo_count = 5; // 스테이지가 올라가면  언두 카운트 초기화
    move_count = 0; // 스테이지가 올라가면 무브 카운트 초기화
  }

  return flag;
}

void Option(char key)
{
  char enter;
  int Top_i;

  // h(H), j(J), k(K), l(L) 를 제외한 키 출력하기
  if(((((((key != 'h' && key != 'j') && key != 'k') && key != 'l') && key != 'H') && key != 'J') && key != 'K') && key != 'L')
    printf("%c", key);

  enter = getch();

  switch (key)
  {
      case 's':
      case 'S':
      if(enter == '\n')
        Save(); // s(S) 키를 눌렀을 때 Save() 기능 실행
        break;

      case 'f':
      case 'F':
      if(enter == '\n')
        FileLoad(); // f(F) 키를 눌렀을 때 FileLoad() 기능 실행
        break;

      case 'd' :
      case 'D' :
        if(d_bl==0)
        {
          d_bl = 1;
          stop_clock = clock(); //흘러가지 않을 시간 시작
          Display();
        }
        StopTime(); //정지된시간 계산
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
          stop_clock = clock(); //흘러가지 않을 시간 시작
        }
        switch (enter)
        {
          // enter 값이 1, 2, 3, 4, 5일 경우 각각의 스테이지 Top 출력
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

          // enter 값이 '\n' 개행일 경우 전체 Top 출력
          case '\n' :
            Top_i = 0; //t만 입력했을때
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
            Top(0); //맵전체 랭킹기록을 보여줌
            break;
          }
          if(Top_i == -1)
          {
            break;
          }
          if(getch() == '\n' && Top_i != 0 && Top_i != -1)
          {
            Top(Top_i); //해당 맵의 랭킹기록을 보여줌
            break;
          }
        }
        StopTime(); //정지된시간 계산
        break;

      case 'e':
      case 'E':
      if(enter == '\n')
        MapClear();
        printf("\n\n\nSEE YOU %s . . . .\n\n\n", username);
        Save(); // 게임 종료하기 전에 저장하기
        exit(0);
        break;

      case 'u':
      case 'U'://u, U 가 입력시 언두 기능 실행
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
  //초기맵과 초기플레이어위치 불러오기
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
  StartTime(); //시작시간 재정의
  MapClear();
  stage = 0; //1스테이지로 변경
  sum_stop = 0; //멈춘시간 초기화

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
  //초기맵과 초기플레이어위치 불러오기
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
            break; //이름이 0.0 디폴트 값으로 되어있으면 배열에 "  "으로 저장한다
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
      break; //파일이 끝나면 더이상 반복하지 않는다.
    }
  }

  //새로운 점수 값이 입력되면 크기를 비교해 배열에 넣는다.
  for(i = 0; i < 4; i++)
  {
    //새로운 점수값이 0~3번쨰 점수들 사이에 들어가거나 0을제외한 값중 가장 클때
    if(score_time[stage][i] > score || score_time[stage][i] == 0 )
    {
      //새로운 점수값이 들어갈자리를 비우기 위해 뒤로 값을 미룬다
      for(j = 0; j < 4-i; j++)
      {
        score_time[stage][4-j] = score_time[stage][3-j];
        for(k = 0; k < 11; k++)
        {
          score_name[stage][4-j][k] = score_name[stage][3-j][k];
        }
      }
      for(k = 0; k < 11; k++)  //새로운 점수값 입력
      {
        score_name[stage][i][k] = username[k];
      }
      score_time[stage][i] = score;
      break;
    }
  }
  //새로운 점수가 4번째값일때
  if(score_time[stage][i] > score || score_time[stage][i] == 0)
  {
    score_time[stage][i] = score;

    for(k = 0; k < 11; k++)
    {
      score_name[stage][i][k] = username[k];
    }
  }

  fclose(fp);

  //랭킹 파일에 저장
  fp = fopen("ranking.txt", "w");

  for(i = 0; i < 5; i++)
  {
    for(j = 0; j < 5; j++)
    {
      if(score_name[i][j][0] == ' ' && score_name[i][j][1] == ' ' && score_name[i][j][2] == '\0')
      {
        fprintf(fp, "0.0 ", score_name[i][j]); //이름이 "  "이면 0.0 디폴트 값으로 저장
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

  //이름과 기록을 읽어들인다.
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
            break; //이름이 0.0 디폴트 값으로 되어있으면 배열에 "  "으로 저장한다
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
      break; //파일이 끝나면 더이상 반복하지 않는다.
    }
  }

  int s = 0; //이름이 저장되어있는지의 유무로 출력하기위한 변수

  //Top_i에 따라 출력
  while(1)
  {
    MapClear(); //0일때 모든맵의 기록 출력
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
              break; //이름이 "  "으로 기록이 없으면 출력하지 않게 함
            }
            else
            {
              s = 1;
              break;
            }
          }
          if(s == 1) //기록이 있을경우 출력
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
            break; //이름이 "  "으로 기록이 없으면 출력하지 않게 함
          }
          else
          {
            s = 1;
            break;
          }
        }
        if(s == 1) //기록이 있을경우 출력
        {
          printf("%s  ", score_name[i][j]);
          printf("%.1f sec", score_time[i][j]);
        }
        printf("\n");
      }
    }
    printf("나가려면 t키를 누르시오."); //출력후 들어온 상태면서 t,T 를 누를경우 빠져나간다.
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
  start_clock = clock();  //시작시간 재정의
}

void EndTime()
{
  end_clock = clock();

  diff_clock = (double)(end_clock - start_clock) / 1000; //전체 시간 측정
  score = diff_clock - sum_stop; //저체시간 - 정지된 동안의 시간
}

void StopTime()
{
  end_clock = clock();

  sum_stop += (double)(end_clock - stop_clock) / 1000; //정지된 동안의 시간측정
}
// 언두 맵 저장
void UndoMap()
{
  int i, j, k = 0;
  // 언두 맵 저장횟수가 5번이 넘었을경우
  if(save_count >= 5)
  {
    for(i = 1; i < STAGE; i++)
    {
      for(j = 0; j < Y; j++)
      {
        for(k = 0; k < X; k++)
        {
          undo[i - 1][j][k] = undo[i][j][k]; // 마지막 언두 저장맵을 버림
        }
      }
    }
    save_count--;
  }
  // 언두를 위한 맵 저장
  for(j = 0; j < Y; j++)
  {
    for(k = 0; k < X; k++)
    {
      undo[save_count][j][k] = map[stage][j][k];
    }
  }
  save_count++;
}
// 언두 실행
void Undo()
{
  int undo_x = 0, undo_y = 0;

  if((undo_count < 1) || ((5 - undo_count) >= move_count))
    return; // 언두 카운트가 0이거나 무브 카운트가 0인데 언두 가능 횟수가 남아 있을 경우 언두를 실행하지 않음

  undo_count--;
  save_count--;

  for(int i = 0; i < Y; i++)
  {
    for(int j = 0; j < X; j++)
    {
      map[stage][i][j] = undo[save_count][i][j]; // 현재 맵을 언두맵에서 저장한 맵으로 대체함
    }
  }
  // @의 좌표값을 저장함
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

  player_y[stage] = undo_y; // 플레이어 y좌표 조정
  player_x[stage] = undo_x; // 플레이어 x좌표 조정
}

// 메인 함수
int main()
{
  // 이름 입력
  UserName();
  // 맵 로드
  MapLoad();

  // 게임 진행
  while(1)
  {
    MapClear();
    MapDraw();
    PlayerMove();
    EndTime();
  }

  return 0;
}
