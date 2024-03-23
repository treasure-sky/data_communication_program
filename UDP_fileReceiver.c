#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> // 문자열 처리 함수
#include <sys/socket.h>
#include <netinet/in.h> // 인터넷 주소 구조체
#include <arpa/inet.h> // 인터넷 주소 변환 함수

#define PORT 12345
#define GREETING "Greeting"
#define OK "OK"

int main(void){
  struct sockaddr_in sender_addr, receiver_addr; // 발신자, 수신자 정보 구조체
  struct timeval tv;
  int sock, receive_len, sock_len = sizeof(receiver_addr), packet_order = 1;
  char receive_message[256], file_name[256];
  FILE *file;

  // 소켓 생성
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket generate error");
    exit(1);
  }

  // timeout 설정
  tv.tv_sec = 60;
  tv.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("set timeout error");
  }

  memset(&receiver_addr, 0, sizeof(receiver_addr)); // 구조체 0으로 초기화
  receiver_addr.sin_family = AF_INET; // IPv4로 설정
  receiver_addr.sin_port = htons(PORT); // 포트를 bigEndian(네트워크 byte order)으로 변환 후 설정
  receiver_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 주소에서 들어오는 요청 받음

  // 소켓에 주소 할당
  if (bind(sock, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) == -1) {
      perror("binding error");
      exit(1);
  }

  printf("수신 대기중입니다...\n");
  
  if ((receive_len = recvfrom(sock, receive_message, sizeof(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len)) == -1) {
    perror("receive greeting error");
    exit(1);
  }

  // Greeting 판별
  receive_message[receive_len] = '\0'; // 전송받은 데이터를 문자열로 인식하기 위해 null값 추가
  if (strcmp(receive_message, "Greeting") != 0) {
    perror("not Greeting");
    exit(1);
  }

  // OK 응답
  if (sendto(sock, OK, strlen(OK), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1) {
    perror("OK send error");
    exit(1);
  }

  // 파일 이름 수신
  if ((receive_len = recvfrom(sock, receive_message, sizeof(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *)&sock_len)) == -1) {
      perror("file name receive error");
      exit(1);
  }

  memcpy(file_name, receive_message, receive_len); // 파일 이름 복사
  printf("받아올 파일 이름: %s\n", file_name);

  // 파일 이름 전송(응답)
  if (sendto(sock, file_name, strlen(file_name), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1) {
    perror("OK send error");
    exit(1);
  }

  // 새로운 파일 만들기
  file = fopen(file_name, "wb");
  if (file == NULL) {
    perror("create file error");
    exit(1);
  }

  while (1) {
    char buffer[512 + sizeof(int)];
    int next_bytes = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len);
    if(next_bytes > 0) {
      int now_order; // 전송 받은 현재 패킷의 순서
      memcpy(&now_order, buffer, sizeof(int));
      if(now_order == 0){ // 전송이 끝났으면 종료
        break;
      }

      // 패킷이 순서가 맞는지
      if(packet_order != now_order){
        perror("packet loss error");
        exit(1);
      }

      if(sendto(sock, buffer, sizeof(int), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1){
        perror("send ack num err");
        exit(1);
      }

      // 파일에 쓰기
      if (fwrite(buffer + sizeof(int), 1, next_bytes - sizeof(int), file) != (int)(next_bytes - sizeof(int))) {
        perror("write file error");
        exit(1);
      }
      printf("%d 번째 패킷 수신 완료\n", now_order);
      packet_order++;
    }
  }

  fclose(file);
  close(sock);
  return 0;
}

