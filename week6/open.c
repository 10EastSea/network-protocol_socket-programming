#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    int fd, n, ret;

    char buf[128];
    struct timeval tv;

    fd_set readfs;
    fd = open("write.txt", O_RDONLY); // 파일 디스크립터 번호 받음

    if(fd < 0) {
        perror("file open error");
        exit(1);
    }

    memset(buf, 0x00, 128);

    FD_ZERO(&readfs);
    while(1) {
        FD_SET(fd, &readfs);
        ret = select(fd+1, &readfs, NULL, NULL, NULL); // 몇개를 감시? -> 0부터 시작하므로 fd+1개 만큼 감시

        if(ret == -1) {
            perror("select error");
            exit(1);
        }

        if(FD_ISSET(fd, &readfs)) {
            while((n = read(fd, buf, 128)) > 0) printf("%s", buf); // 읽을게 없을 때까지 반복 -> 백그라운드에서 실행
        }
        
        memset(buf, 0x00, 128); // 버퍼 한번 돌았으므로 다시 초기화
        usleep(1000); // 1초 쉬었다가 진행
    }
}