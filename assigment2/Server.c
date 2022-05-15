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

int clnt_socks[2];
char win_msg[BUF_SIZE] = "당신이 이겼습니다\n";
char lose_msg[BUF_SIZE] = "당신은 졌습니다\n";

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz; // int clnt_adr_sz;

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
        if(status == 0) { // 빙고! -> 1번째 플레이어 승
            printf("1번째 플레이어 빙고!\n");
            write(clnt_socks[0], &win_msg, BUF_SIZE);
            write(clnt_socks[1], &lose_msg, BUF_SIZE);
            break;
        }
        status = gaming_handler(1); // 2번째 플레이어에게 숫자 받고, 1번째 플레이어에게 숫자 전송
        if(status == 0) { // 빙고! -> 2번째 플레이어 승
            printf("2번째 플레이어 빙고!\n");
            write(clnt_socks[1], &win_msg, BUF_SIZE);
            write(clnt_socks[0], &lose_msg, BUF_SIZE);
            break;
        }
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

    int status = 1;
    if(bingo_cnt >= 3) status = 0;

    if(clnt == 0) {
        printf("1번째 플레이어: Call %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
        write(clnt_socks[1], (char*)&number, sizeof(int));
        write(clnt_socks[1], (char*)&status, sizeof(int));
    }
    else if(clnt == 1) {
        printf("2번째 플레이어: Call %2d / 현재 빙고 갯수 = %d개\n", number, bingo_cnt);
        write(clnt_socks[0], (char*)&number, sizeof(int));
        write(clnt_socks[0], (char*)&status, sizeof(int));
    }

    return status;
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}