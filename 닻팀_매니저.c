// 선언 헤더 파일
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>

#include <net/if.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>

// 정의 값
#define BUF_SIZE 500
#define TIMEOUT 5

//요청한 클라이언트 객체의 구조체
typedef struct client {
  char ip_address[20];
  char name[300];
  char status[2]; //이때 스테이터스 0 은 비접속 미채팅 1은 접속 미채팅 2는 접속 채팅
} CLIENT;

void* thread1(){
	int sockfd;
    struct sockaddr_in servAddr;
    struct sockaddr_in clntAddr;
    char recvBuffer[BUF_SIZE];
    int clntLen;
    int recvLen;
	char PORT[4] ="1500";
	CLIENT  client[255];

    /* 인터넷으로 연결된 프로세스들 간에 통신을 하고 UDP 방법을 이용하는 소켓을 생성 */
    if((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
       perror("sock failed");
       exit(1);
    }

    /* servAddr를 0으로 초기화 */
    memset(&servAddr, 0, sizeof(servAddr));
    /* servAddr에 IP 주소와 포트 번호를 저장 */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(PORT));

    /* sockfd 소켓에 주소 정보 연결 */
    if(bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1) {
       perror("bind failed");
       exit(1);
    }
    int cnt =0;
	int stat =0;
	char temp[20]="0";
    /* 무한 반복 */
    while(1) {
       clntLen = sizeof(clntAddr);
       /* sockfd 소켓으로 들어오는 데이터를 받아 recvBuffer에 저장하고
          클라이언트 주소 정보를 clntAddr에 저장 */
       if((recvLen=recvfrom(sockfd, recvBuffer, BUF_SIZE-1, 0, (struct sockaddr*)&clntAddr, &clntLen)) == -1) {
          perror("recvfrom failed");
          exit(1);
       }
       recvBuffer[recvLen] = '\0';
       /* 받은 데이터를 출력 */
       printf("Recevied: %s\n", recvBuffer);

       /* 받은 데이터를 클라이언트에게 보냄 
       if(sendto(sockfd, recvBuffer, recvLen, 0, (struct sockaddr*)&clntAddr, sizeof(clntAddr)) != recvLen) {
          perror("sendto failed");
          exit(1);
       }*/
		if(strcmp(recvBuffer,"@show_clients")==0)stat=4;
		
	    
		if(stat==1){
		  strcpy(client[cnt].name,recvBuffer);
		  stat=0;
		  char temp[2];
		  sprintf(temp, "%d", cnt);
		  sendto(sockfd,temp, recvLen, 0, (struct sockaddr*)&clntAddr, sizeof(clntAddr));
	    }
		if(stat ==2){
		 strcpy(client[cnt].ip_address,recvBuffer);
		 stat=0;
		 if(strcmp("0",temp)==0){
			 strcpy(temp,client[cnt].ip_address);
		 }
		 if(strcmp(client[cnt].ip_address,temp)!=0){
			 strcpy(temp,client[cnt].ip_address);
			 cnt++;
		 }
		}
		if(stat==3){
		  char key[2];
		  char status[2];
		  memcpy(&key,&recvBuffer,1);
		  memcpy(&status,&recvBuffer[1],1);
		  strcpy(client[atoi(key)].status,status);
		  stat=0;
	    }
		if(stat==4){
			int n=0;
			char msg[1000] = "●";
			char link[100] = " ";
			sendto(sockfd,"전체 접속자 현황", recvLen, 0, (struct sockaddr*)&clntAddr, sizeof(clntAddr));
			while (n<=cnt){
				strcat(msg,link);
				strcat(msg,client[n].name);
				strcat(msg,link);
				strcat(msg,client[n].ip_address);
				strcat(msg,link);
				if (strcmp(client[n].status, "1")==0)	strcat(msg , " 연결 가능 접속중");
				else if (strcmp(client[n].status, "0")==0)	strcat(msg , " 미접속중");
				else if (strcmp(client[n].status ,"2")==0)strcat(msg , " 다른 상대와 연결중");
				else strcat(msg , "ERROR");
				strcat(msg,link);
				sendto(sockfd,msg, 1000, 0, (struct sockaddr*)&clntAddr, sizeof(clntAddr));
				memset(msg, 0, 1000);
				n++;
			}
			sendto(sockfd,"-------", recvLen, 0, (struct sockaddr*)&clntAddr, sizeof(clntAddr));
		  stat=0;
	    }
		
		if(strcmp(recvBuffer,"@name")==0)stat=1;
	    if(strcmp(recvBuffer,"@ip")==0)stat=2;
		if(strcmp(recvBuffer,"@heartbeat")==0)stat=3;
    }
	close(sockfd);
}

void error_handling(char * message);

//개인 IP주소 참조 함수
void GetMyIpAddr(char *ip_buffer)
{
    int fd;
    struct ifreq ifr;
 
    fd = socket(AF_INET, SOCK_DGRAM, 0);
     
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "enp0s8", IFNAMSIZ);
 
    if(ioctl(fd, SIOCGIFADDR, &ifr)<0){
		printf("error\n");
	}

    struct sockaddr_in *sin;
     
   inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ip_buffer,sizeof(struct sockaddr));
   close(fd);
}


//메인 함수
int main()
{	
	
	
	char mbuf[20];//멀티메세지 버퍼
	char PORT[4] = "1500";
	struct timeval tv;//시간 변수
	fd_set readfds;//시간읽기
	int ret; //대기조건
	char ipstr[20]={0}; //IP 저장소
	GetMyIpAddr(ipstr);// 자기 IP 주소를 가져오는 함수를 통해 저장
	strcpy(mbuf,ipstr); //IP 주소 복사

	
	pthread_t thread;
	int new_thread;
	new_thread = pthread_create(&thread,NULL,thread1,NULL);
	if(new_thread<0) printf("쓰레드 에러\n");
	
	
	 /*멀티케스트 메세지 연결 코드 -------*/
	//멀티케스트 아이피 메세지 전송용 공용 아이피와 포트
	char mult_ip[20]="239.0.120.1";
	int mult_port = 1557;
	int msend_sock;//멀티메세지 소켓 선언
	int time_live = 5;//TTL 생존 시간
	struct sockaddr_in adr, sadr;// 소켓 에드레스 버퍼 
    struct ip_mreq join_adr; // 주소 접속 에드레스
	msend_sock=socket(PF_INET, SOCK_DGRAM, 0);
	sadr.sin_family=AF_INET;
	sadr.sin_addr.s_addr=inet_addr(mult_ip);
	sadr.sin_port=htons(mult_port);
	setsockopt(msend_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&time_live, sizeof(time_live));
    /* 연결완료 ------------------------------*/	
	
	while(1){
		
		
	/* 반복해서 서버 차원에서 자신의 IP 주소를 메세지로 보냄*/
	
	// 표준 입력에서 입력 대기
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    // select가 5초 동안 기다리도록 timeval 구조체 설정
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    
    // select() 시스템콜을 이용해 입력 대기
    ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);  
    if (ret == -1) perror("select");
    else if (!ret) {
	sendto(msend_sock, mbuf, 20, 0,(struct sockaddr *)&sadr, sizeof(sadr));
	sendto(msend_sock, PORT, 4, 0,(struct sockaddr *)&sadr, sizeof(sadr));

	}
    /*--------------------------------*/
	}
	
	/*마지막으로 모든 소켓들을 종료한다.*/
	close(msend_sock);

}

void error_handling(char * message){
 fputs(message, stderr);
 fputc('\n',stderr);
 exit(1);
}


	  
		
		
	   