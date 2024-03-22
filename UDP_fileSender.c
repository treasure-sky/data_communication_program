#include <stdio.h> // 표준입출력
#include <stdlib.h> // 유틸리티 함수
#include <unistd.h> // 유닉스 시스템 호출
#include <string.h> // 문자열 처리 함수
#include <sys/socket.h> // 소켓관련 함수, 구조체
#include <netinet/in.h> // 인터넷 주소 구조체
#include <arpa/inet.h> // 인터넷 주소 변환 함수

#define GREETING "Greeting"
#define OK "OK"

int main(void){
  struct sockaddr_in receiver_addr; // 수신자 정보 구조체
  int sock, receive_len, sock_len = sizeof(receiver_addr), packet_order = 0;
  char receive_message[256], ip[16], file_name[256], file_ack[32];
  unsigned short int port;

  // ip, port 입력
  printf("파일을 전송 받을 IP주소를 입력해주세요: ");
  if(fgets(ip, sizeof(ip), stdin) == NULL){
    perror("input IP format error");
    exit(1);
  }
  ip[strcspn(ip, "\n")] = 0;

  printf("파일을 전송 받을 PORT번호를 입력해주세요: ");
  if(scanf("%hu", &port) == 0){
    perror("input PORT format error");
    exit(1);
  }

  // 파일 이름 입력
  printf("파일의 이름을 입력해주세요(해당 프로그램과 같은 위치에 존재해야 합니다): ");
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

  memset(&receiver_addr, 0, sock_len); // 구조체 0으로 초기화
  receiver_addr.sin_family = AF_INET; // IPv4로 설정
  receiver_addr.sin_port = htons(port); // remote port를 bigEndian(네트워크 byte order)으로 변환 후 설정

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
  if ((receive_len = recvfrom(sock, receive_message, strlen(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len)) == -1) {
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
  if ((receive_len = recvfrom(sock, receive_message, strlen(receive_message), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len)) == -1) {
    perror("file name error");
    exit(1);
  }

  // 파일 이름 메세지 판별
  receive_message[receive_len] = '\0'; // 전송받은 데이터를 문자열로 인식하기 위해 null값 추가
  if (strcmp(receive_message, file_name) != 0) {
    perror("not same file name");
    exit(1);
  }

  while (!feof(file))
  {
    char buffer[512]; // udp 패킷의 안전 범위
    int next_bytes = fread(buffer, 1, sizeof(buffer), file);
    if(next_bytes > 0) { // file에 내용이 아직 존재하면
      char packet[516];
      memcpy(packet, &packet_order, sizeof(int)); // packet 맨 앞에 패킷순서(packet_order)삽입
      memcpy(packet + sizeof(int), buffer, next_bytes); // 데이터 삽입

      while(1){
        // 패킷 전송
        if(sendto(sock, packet, next_bytes + sizeof(int), 0, (struct sockaddr *)&receiver_addr, sock_len) == -1){
          perror("packet sending error");
          exit(1);
        }

        // receiver 응답 대기
        memset(file_ack, 0, sizeof(file_ack)); // 버퍼 초기화
        if(recvfrom(sock, file_ack, sizeof(file_ack), 0, (struct sockaddr *) &receiver_addr, (socklen_t *) &sock_len) == -1){
          perror("ack error");
          exit(1);
        }
        
        int now_order;
        memcpy(&file_ack, &now_order, sizeof(int));
        if(now_order != packet_order){
          printf("not same packet");
        } else{
          break;
        }
      }
      packet_order++;
      
    }
  }
  fclose(file);
  close(sock);
  printf("file sended.");
  return 0;

}

