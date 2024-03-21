#include <stdio.h> // 표준입출력
#include <stdlib.h> // 유틸리티 함수
#include <unistd.h> // 유닉스 시스템 호출
#include <string.h> // 문자열 처리 함수
#include <sys/socket.h> // 소켓관련 함수, 구조체
#include <netinet/in.h> // 인터넷 주소 구조체
#include <arpa/inet.h> // 인터넷 주소 변환 함수

#define IP "192.168.0.1"
#define PORT 1234
#define GREETING "Greeting"
#define OK "OK"

int main(void){
  struct sockaddr_in server_addr; // 수신자 정보 구조체
  int sock, receive_len; // 소켓, 응답문자 개수
  char receive_message[4];
  
  // 소켓 생성
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket generate error");
    exit(1);
  }

  memset(&server_addr, 0, sizeof(server_addr)); // 구조체 0으로 초기화
  server_addr.sin_family = AF_INET; // IPv4로 설정
  server_addr.sin_port = htons(PORT); // 포트를 bigEndian(네트워크 byte order)으로 변환 후 설정

  // IP를 bigEndian(네트워크 byte order)으로 변환 후 설정
  if (inet_aton(IP, &server_addr.sin_addr) == 0) { 
    perror("IP format error");
    exit(1);
  }

  // Greeting 전송
  if (sendto(sock, GREETING, strlen(GREETING), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("greeting error");
    exit(1);
  }

  // OK 대기
  if ((receive_len = recvfrom(sock, receive_message, strlen(receive_message), 0, (struct sockaddr *) &server_addr, sizeof(server_addr))) == -1) {
    perror("ok error");
    exit(1);
  }

  // OK 판별
  receive_message[receive_len] = '\0'; // 전송받은 데이터를 문자열로 인식하기 위해 null값 추가
  if (strcmp(receive_message, "OK") != 0) {
    perror("not OK");
    exit(1);
  }

  printf("OK");

  close(sock);
  return 0;

}
