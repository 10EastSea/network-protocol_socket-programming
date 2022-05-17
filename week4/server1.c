#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h> // 다중 클라이언트를 처리하기 위해 붙는 헤더

#define BUF_SIZE 100
void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;

    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if(argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1) error_handling("bind() ERROR");
    if(listen(serv_sock, 5) == -1) error_handling("listen() ERROR");

    FD_ZERO(&reads); // reads를 초기화
    FD_SET(serv_sock, &reads); // reads의 주소값을 serv_sock에 넣어서 관찰을 하겠다는 의미
    fd_max = serv_sock; // serv_sock도 시스템 프로세스로 떨어짐

    while(1) {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        // fd_max 값에서 1을 증가 (파일 디스크립터 넘버 자리 <- 0하고 1이 들어감 => 0이면 관찰 X, 1이면 관찰)
        if((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1) break; // 에러가 났을 때
        if(fd_num == 0) continue; // 에러가 나지 않았을 때

        for(i=0; i<fd_max+1; i++) {
            if(FD_ISSET(i, &cpy_reads)) {
                if(i == serv_sock) {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*) &clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if(fd_max < clnt_sock) fd_max = clnt_sock;
                    printf("connected client: %d\n", clnt_sock);
                } else {
                    str_len = read(i, buf, BUF_SIZE);
                    if(str_len == 0) {
                        FD_CLR(i, &reads); // 관찰을 종료하겠다 (정보를 삭제하겠다)
                        close(i); // 번호로 닫아줌
                        printf("close client: %d\n", i);
                    } else { write(i, buf, str_len); }
                }
            }
        }
    }

    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}