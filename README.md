# 파일 전송 프로그램

이 프로그램은 리눅스환경에서 UDP프로토콜을 통해 파일을 전송하는 프로그램입니다.
UDP_fileSender.c: 파일 송신 프로그램
UDP_fileReceiver.c: 파일 수신 프로그램

## 컴파일 방법
gcc UDP_fileSender.c -o UDP_fileSender
gcc UDP_fileReceiver.c -o UDP_fileReceiver

위 명령을 통해 실행파일을 생성할 수 있습니다.

## 실행 순서
1. UDP_fileReceiver를 파일을 받을 전송받을 환경에서 실행해 주세요 ex ./UDP_fileReceiver
2. UDP_fileReceiver가 수신 대기중입니다... 를 출력합니다.
3. UDP_fileSender를 파일을 전송할 환경에서 실행해주세요.
4. IP를 입력하라는 말이 출력되면 수신자의 ip를 입력해주세요. ex. 127.0.0.1
5. 보내려는 파일의 이름을 입력해달라는 말이 출력되면 파일의 이름을 입력해주세요. ex. test.pdf

## 주의 사항
**실행한 UDP_fileSender 파일과 동일 위치에 존재하는 파일의 이름을 입력해주세요**
**UDP_fileReceiver는 실행 후 1분동안 동작하므로 실행 후 1분 내에 UDP_fileSender를 통해 데이터를 전송해 주세요**


