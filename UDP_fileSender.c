#include <stdio.h> // 표준입출력
#include <stdlib.h> // 유틸리티 함수
#include <unistd.h> // 유닉스 시스템 호출
#include <string.h> // 문자열 처리 함수
#include <sys/socket.h> // 소켓관련 함수, 구조체
#include <netinet/in.h> // 인터넷 주소 구조체
#include <arpa/inet.h> // 인터넷 주소 변환 함수

#define PORT 1234
#define IP "192.168.0.1"

int main(void){
  struct sockaddr_in addr; // 수신자 정보 구조체
  int sock;
  char message[] = "Greeting";
  
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket generate error");
    exit(1);
  }

  // memset((char *)&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET; // IPv4로 설정
  addr.sin_port = htons(PORT); // 포트를 bigEndian(네트워크 byte order)으로 변환 후 설정

  if (inet_aton(IP, &addr.sin_addr) == 0) { // IP를 bigEndian(네트워크 byte order)으로 변환 후 설정
    perror("IP format error");
    exit(1);
  }

  if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("sendto error");
    exit(1);
  }

  close(sock);
  return 0;

}

// char* getIP(){
//   static char ip[16];

//   int hostname;
//   // hostname = gethostname();
  
// }


/**
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 9876

int main(void) {
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    char buf[BUFLEN];
    char message[] = "Hello, Server!";

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);

    if (inet_aton(SERVER, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    if (sendto(s, message, strlen(message), 0, (struct sockaddr *)&si_other, slen) == -1) {
        perror("sendto()");
        exit(1);
    }

    close(s);
    return 0;
}
**/