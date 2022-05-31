#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char* msg);

int recv_status();
void send_status_all(int status);
void send_status_each_other(int status1, int status2);

int gaming_handler(int clnt);
void finish_handler(int status);

int clnt_socks[2];
char win_msg[BUF_SIZE] = "당신이 이겼습니다\n";
char lose_msg[BUF_SIZE] = "당신은 졌습니다\n";
char draw_msg[BUF_SIZE] = "무승부 입니다\n";

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc != 2) {
        printf("Usage: %s <PORT>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1) error_handling("bind() ERROR");
    if(listen(serv_sock, 5) == -1) error_handling("listen() ERROR");
    printf("Server open...\n");

    // 1번째 플레이어
    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_sz);
    clnt_socks[0] = clnt_sock;
    printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

    // 2번째 플레이어
    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &clnt_adr_sz);
    clnt_socks[1] = clnt_sock;
    printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));

    send_status_all(1);
    printf("게임 시작!\n");

    int status = recv_status(); // 빙고판 생성 대기
    if(status != 1) error_handling("read ERROR()");
    send_status_each_other(1, 0); // 빙고 시작 -> 먼저 접속한 플레이어에게 선두권
    while(1) {
        status = gaming_handler(0); // 1번째 플레이어에게 숫자 받고, 2번째 플레이어에게 숫자 전송
        finish_handler(status);
        if(status != 0) break;

        status = gaming_handler(1); // 2번째 플레이어에게 숫자 받고, 1번째 플레이어에게 숫자 전송
        finish_handler(status);
        if(status != 0) break;
    }

    close(clnt_socks[0]);
    close(clnt_socks[1]);
    close(serv_sock);
    return 0;
}

int recv_status() {
    int statuses[2] = {0, 0};
    for(int i=0; i<2; i++) read(clnt_socks[i], (char*)&statuses[i], sizeof(int));
    
    if(statuses[0] == 1 && statuses[1] == 1) return 1;
    return 0;
}
void send_status_all(int status) {
    for(int i=0; i<2; i++) write(clnt_socks[i], (char*)&status, sizeof(int));
}
void send_status_each_other(int status1, int status2) {
    write(clnt_socks[0], (char*)&status1, sizeof(int));
    write(clnt_socks[1], (char*)&status2, sizeof(int));
}

int gaming_handler(int clnt) {
    int number = 0; int bingo_cnt = 0;
    read(clnt_socks[clnt], (char*)&number, sizeof(int));
    read(clnt_socks[clnt], (char*)&bingo_cnt, sizeof(int));

    int status = 0; // 0: 게임중, 1: 1번째 플레이어 승, 2: 2번째 플레이어 승, 3: 무승부
    if(bingo_cnt >= 3) {
        if(clnt == 0) status = 1;
        else if(clnt == 1) status = 2;
    }

    if(clnt == 0) {
        printf("1번째 플레이어: Call %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
        write(clnt_socks[1], (char*)&number, sizeof(int));
        
        read(clnt_socks[1], (char*)&bingo_cnt, sizeof(int));
        if(bingo_cnt >= 3) { // 2번째 플레이어도 빙고 3개 완성
            if(status == 0) status = 2;
            else if(status == 1) status = 3; // 무승부
        }
        printf("2번째 플레이어: Recv %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
    }
    else if(clnt == 1) {
        printf("2번째 플레이어: Call %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
        write(clnt_socks[0], (char*)&number, sizeof(int));

        read(clnt_socks[0], (char*)&bingo_cnt, sizeof(int));
        if(bingo_cnt >= 3) { // 1번째 플레이어도 빙고 3개 완성
            if(status == 0) status = 1;
            else if(status == 2) status = 3; // 무승부
        }
        printf("1번째 플레이어: Recv %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
    }

    write(clnt_socks[0], (char*)&status, sizeof(int));
    write(clnt_socks[1], (char*)&status, sizeof(int));
    return status;
}

void finish_handler(int status) {
    if(status == 1) { // 빙고! -> 1번째 플레이어 승
        printf("1번째 플레이어 빙고!\n");
        write(clnt_socks[0], &win_msg, BUF_SIZE);
        write(clnt_socks[1], &lose_msg, BUF_SIZE);
    } else if(status == 2) { // 빙고! -> 2번째 플레이어 승
        printf("2번째 플레이어 빙고!\n");
        write(clnt_socks[1], &win_msg, BUF_SIZE);
        write(clnt_socks[0], &lose_msg, BUF_SIZE);
    } else if(status == 3) { // 빙고! -> 무승부
        printf("1번재, 2번째 플레이어 빙고!\n");
        write(clnt_socks[0], &draw_msg, BUF_SIZE);
        write(clnt_socks[1], &draw_msg, BUF_SIZE);
    }
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}