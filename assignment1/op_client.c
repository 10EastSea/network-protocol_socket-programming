#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // struct sockaddr_in 정의
#include <sys/socket.h> // socket() 정의

#define BUF_SIZE 1024
#define RLT_SIZE 4
#define OPSZ 4

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    char opmsg[BUF_SIZE]; // 하나의 배열로 보낼 예정
    int result, opnd_cnt, i; // opnd_cnt: 몇개의 인자를 넣을 것인지
    struct sockaddr_in serv_adr;

    if(argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1) error_handling("socket() ERROR");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) error_handling("connect() ERROR");
    else puts("Connect--->");

    fputs("Operand count: ", stdout);
    scanf("%d", &opnd_cnt);
    opmsg[0] = (char)opnd_cnt; // opmsg 첫번째에 갯수를 넣음

    for(i=0; i<opnd_cnt; i++) {
        printf("Operand %d: ", i+1);
        scanf("%d", (int*)&opmsg[i*OPSZ + 1]); // 피연산자 하나 받아서 opmsg에 넣음 (+1은 opnd_cnt와 겹치지 않게 하기 위함)
    }

    fgetc(stdin);
    fputs("Operator: ", stdout);
    scanf("%c", &opmsg[opnd_cnt*OPSZ + 1]);

    write(sock, opmsg, opnd_cnt*OPSZ + 2);
    read(sock, &result, RLT_SIZE);

    printf("Operation result: %d\n", result);

    close(sock);
    return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
