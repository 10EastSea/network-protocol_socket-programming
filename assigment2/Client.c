#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

#define BUF_SIZE 100
void error_handling(char* msg);

int create_bingo();
int call_number();

void check_bingo();
void update_bingo(int number);
void print_bingo();

char msg[BUF_SIZE];
int bingo[5][5]; // 빙고판
int bingo_cnt = 0;

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int sock;
    struct sockaddr_in serv_addr;
    void *thread_return;

    int status = 0;

    if(argc != 3) {
        printf("Usage : %s <IP> <PORT>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() ERROR");

    // 1. 다른 플레이어 기다리기
    printf("다른 플레이어를 기다리는 중...\n");
    read(sock, &status, sizeof(int));
    if(status != 1) error_handling("read ERROR()");
    else printf("게임을 시작합니다\n");

    // 2. 빙고판 생성하기
    status = 0;
    while(status != 1) status = create_bingo(); // 빙고판 생성
    write(sock, (char*)&status, sizeof(int));
    printf("상대방 응답 대기 중...\n");

    // 3. 본격적인 게임 시작
    read(sock, &status, sizeof(int));
    printf("\n========== START ==========\n");
    while(status != -1) {
        printf("\n현재 내 빙고판\n");
        print_bingo();
        printf("빙고 갯수: %d\n", bingo_cnt);

        int number = 0;
        if(status == 1) {
            printf("\n당신 차례 입니다\n");
            number = call_number();

            update_bingo(number);

            write(sock, (char*)&number, sizeof(int));
            write(sock, (char*)&bingo_cnt, sizeof(int));
            status = 0;
        }
        else if(status == 0) {
            printf("\n상대방 응답 대기 중...\n");
            read(sock, &number, sizeof(int));
            read(sock, &status, sizeof(int));

            printf("상대방이 부른 숫자: %d\n", number);
            update_bingo(number);
            
            if(status == 0) break; // 상대방이 이겼을 때
            status = 1;
        }

        if(bingo_cnt >= 3) break; // 내가 이겼을 때
    }
    printf("==========  END  ==========\n");
    printf("\n최종 빙고판\n");
    print_bingo();
    printf("빙고 갯수: %d\n", bingo_cnt);
    read(sock, &msg, BUF_SIZE);
    fputs(msg, stdout);

    close(sock);
    return 0;
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

////////////////////////////////////////////////////////////////////////////////

int create_bingo() {
    printf("\n5X5 빙고판을 작성해주세요\n");
    printf("1~25까지의 숫자를 띄어쓰기로 구분하여, 한 줄로 25개의 숫자를 입력하면 됩니다\n");
    printf("ex) 5 21 17 8 6 ... 1 22 13\n");
    printf("만약 랜덤으로 빙고판을 생성하길 원한다면 0을 입력하세요\n");
    printf(">> ");

    fgets(msg, BUF_SIZE, stdin);
    if(!strcmp(msg, "0\n")) {
        int tmps[25];
        for(int i=0; i<25; i++) {
            tmps[i] = (rand() % 25) + 1;
            for(int j=0; j<i; j++) { if(tmps[j] == tmps[i]) { i--; break; } }
        }
        for(int i=0; i<5; i++) {
            for(int j=0; j<5; j++) { bingo[i][j] = tmps[5*i + j]; }
        }
    } else {
        char* number = strtok(msg, " ");
        for(int i=0; i<5; i++) {
            for(int j=0; j<5; j++) {
                if(number == NULL) {
                    printf("25개의 숫자를 입력해야 합니다\n");
                    return -1;
                }
                bingo[i][j] = atoi(number);
                number = strtok(NULL, " ");
            }
        }
    }

    print_bingo();
    printf("이 빙고판으로 하시겠습니까? (예: 1, 아니오: 2)\n");

    fgets(msg, BUF_SIZE, stdin);
    if(!strcmp(msg, "1\n")) return 1;
    else return 0;
}

int call_number() {
    printf("숫자를 선택하세요\n");
    printf(">> ");

    fgets(msg, BUF_SIZE, stdin);
    return atoi(msg);
}

void check_bingo() {
    bingo_cnt = 0;
    int row[5] = {0, 0, 0, 0, 0};
    int col[5] = {0, 0, 0, 0, 0};
    int dia[2] = {0, 0};

    for(int i=0; i<5; i++) {
        for(int j=0; j<5; j++) {
            if(bingo[i][j] == 0) {
                row[i]++;
                col[j]++;
                if(i == j) dia[0]++;
                if(i+j == 4) dia[1]++;
            }
        }
    }

    for(int i=0; i<5; i++) {
        if(row[i] == 5) bingo_cnt++;
        if(col[i] == 5) bingo_cnt++;
    }
    if(dia[0] == 5) bingo_cnt++;
    if(dia[1] == 5) bingo_cnt++;
}
void update_bingo(int number) {
    for(int i=0; i<5; i++) {
        for(int j=0; j<5; j++) {
            if(bingo[i][j] == number) bingo[i][j] = 0;
        }
    }
    check_bingo();
}
void print_bingo() {
    printf("┏━━━━━━━━━━━━━━━━┓\n");
    for(int i=0; i<5; i++) {
        printf("┃");
        for(int j=0; j<5; j++) {
            if(bingo[i][j] != 0) printf("%3d", bingo[i][j]);
            else printf("  X");
        }
        printf(" ┃\n");
    }
    printf("┗━━━━━━━━━━━━━━━━┛\n");
}