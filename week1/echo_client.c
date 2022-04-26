#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // struct sockaddr_in 정의
#include <sys/socket.h> // socket() 정의

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]) {
	int sock;
	char message[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

	if(argc != 3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	sock = socket(PF_INET, SOCK_STREAM, 0); // socket 정의 (TCP 통신 / 프로토콜에 정의)
	if(sock == -1) error_handling("socket() ERROR");

	memset(&serv_adr, 0, sizeof(serv_adr)); // serv_adr 0으로 초기화
	serv_adr.sin_family = AF_INET; // TCP 통신 (나 자신을 정의)
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]); // 규격 맞춤(??)
	serv_adr.sin_port = htons(atoi(argv[2])); // 포트설정

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) error_handling("connect() ERROR");
	else puts("connect...");

	while(1) {
		fputs("Input message(Q, q to end): ", stdout);
		fgets(message, BUF_SIZE, stdin);

		if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")) break;

		write(sock, message, strlen(message)); // 소켓에서 메시지 보냄
		str_len = read(sock, message, BUF_SIZE-1); // 메시지 받음
		message[str_len] = 0;
		printf("meassage from server %s \n", message);
	}

	close(sock);
	return 0;
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
