#include <stdio.h> // 표준입출력
#include <stdlib.h> // 유틸리티 함수
#include <unistd.h> // 유닉스 시스템 호출
#include <string.h> // 문자열 처리 함수
#include <sys/socket.h> // 소켓관련 함수, 구조체
#include <netinet/in.h> // 인터넷 주소 구조체
#include <arpa/inet.h> // 인터넷 주소 변환 함수

#define PORT 12345

#define GREETING "Greeting"
#define OK "OK"

int main(void){
  struct sockaddr_in receiver_addr; // 수신자 정보 구조체
  struct timeval tv;
  int sock, receive_len, sock_len = sizeof(receiver_addr), packet_order = 1;
  char receive_message[256], ip[16], file_name[256];

  // ip
  printf("파일을 전송 받을 IP주소를 입력해주세요: ");
  if(fgets(ip, sizeof(ip), stdin) == NULL){
    perror("input IP format error");
    exit(1);
  }
  ip[strcspn(ip, "\n")] = 0;


  // 파일 이름 입력
  printf("파일의 이름을 입력해주세요(해당 프로그램과 같은 위치에 존재해야 합니다): ");
  fflush(stdin);
  if(fgets(file_name, sizeof(file_name), stdin) == NULL){
    perror("input file name format error");
    exit(1);
  }
  file_name[strcspn(file_name, "\n")] = 0;

  // 소켓 생성
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket generate error");
    exit(1);
  }

  // timeout 설정
  tv.tv_sec = 10;
  tv.tv_usec = 0;
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("set timeout error");
  }

  memset(&receiver_addr, 0, sock_len); // 구조체 0으로 초기화
  receiver_addr.sin_family = AF_INET; // IPv4로 설정
  receiver_addr.sin_port = htons(PORT); // remote port를 bigEndian(네트워크 byte order)으로 변환 후 설정

  // remote IP를 bigEndian(네트워크 byte order)으로 변환 후 설정
  if (inet_aton(ip, &receiver_addr.sin_addr) == 0) { 
    perror("inet_aton error");
    exit(1);
  }

  // Greeting 전송
  if (sendto(sock, GREETING, strlen(GREETING), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1) {
    perror("greeting error");
    exit(1);
  }

  // OK 대기
  if ((receive_len = recvfrom(sock, receive_message, sizeof(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len)) == -1) {
    perror("ok error");
    exit(1);
  }

  // OK 판별
  receive_message[receive_len] = '\0'; // 전송받은 데이터를 문자열로 인식하기 위해 null값 추가
  if (strcmp(receive_message, "OK") != 0) {
    perror("not OK");
    exit(1);
  }

  // 파일 열기
  FILE *file = fopen(file_name, "rb");
  if(file == NULL) {
    perror("file open error");
    exit(1);
  }

  // 파일 이름 전송
  if (sendto(sock, file_name, strlen(file_name), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1) {
    perror("file name send error");
    exit(1);
  }

  // 파일 이름 도착 확인
  if ((receive_len = recvfrom(sock, receive_message, sizeof(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len)) == -1) {
    perror("file name error");
    exit(1);
  }


  // 파일 이름 메세지 판별
  if (strcmp(receive_message, file_name) != 0) {
    perror("not same file name");
    exit(1);
  }

  while (!feof(file)) {
    char buffer[512]; // udp 패킷의 안전 범위
    int next_bytes = fread(buffer, 1, sizeof(buffer), file);
    if(next_bytes > 0) { // file에 내용이 아직 존재하면
      char packet[512 + sizeof(int)];
      memcpy(packet, &packet_order, sizeof(int)); // packet 맨 앞에 패킷순서(packet_order)삽입
      memcpy(packet + sizeof(int), buffer, next_bytes); // 데이터 삽입

      // 패킷 전송
      printf("%d번째 패킷 전송\n", packet[0]);
      if(sendto(sock, packet, next_bytes + sizeof(int), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1){
        perror("packet sending error");
        exit(1);
      }

      // receiver 응답 대기
      char file_ack[sizeof(int)];
      if(recvfrom(sock, file_ack, sizeof(int), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len) == -1){
        perror("ack error");
        exit(1);
      }

      int ack_num;
      memcpy(&ack_num, file_ack, sizeof(int));
      if (ack_num != packet_order) {
        perror("not same file name");
        exit(1);
      }
      
      packet_order++;
    }
    
    
  }

  int temp = 0;
  if(sendto(sock, &temp, sizeof(int), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1){
    perror("inform last packet error");
    exit(1);
  }
  
  fclose(file);
  close(sock);
  printf("file sended.");
  return 0;

}

