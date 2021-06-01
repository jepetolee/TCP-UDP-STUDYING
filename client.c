// 선언 헤더 파일
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>

#include <arpa/inet.h>
#include <net/if.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/in.h>


// 정의 값
#define BUF_SIZE 500
#define TIMEOUT 3

int _kbhit(void)
{
    struct termios oldt, newt;
    int ch;

 

    tcgetattr( STDIN_FILENO, &oldt );
    newt = oldt;

 

    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newt );

 

    ch = getchar();

 

    tcsetattr( STDIN_FILENO, TCSANOW, &oldt );

 

    return ch;
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

/*


void * control(void * arg[]){
	int sock;
	char msg[24] = arg;
	char buf[20];
	char port[4];
	memcpy(port,msg,4);
	
	memcpy(buf,msg[4],20);
	char message[BUF_SIZE]={0, };
	struct sockaddr_in serv_adr;
	 int servLen;
	sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(sock==-1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(buf);
	serv_adr.sin_port=htons(atoi(port));
	while(1){
		if(_kbhit()){
		 char command[40];
		 char msg[100];
		 fgets(command,1000,stdin);
		 if(strcmp(command,"@show_clients")==0){
			  sendto(sock, "@show_clients", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
			  while(strcmp(msg,"-------")!=0){
				  recvfrom(sock,msg,10,0,(struct sockaddr *)&serv_adr,&servLen);
				  fputs(msg,stdout);
			  }
		 }
		}
	}
}*/

//메인 함수
int main()
{
	//멀티케스트 아이피 메세지 전송용 공용 아이피와 포트
	char mult_ip[20]="239.0.120.1";
	int mult_port = 1557;
	
	int mrecv_sock;//멀티메세지 소켓 선언
	int str_len;
	char buf[BUF_SIZE];
	struct sockaddr_in addr;
	struct ip_mreq join_addr;
	
	
   /*---멀티케스트 연결 코드 ----------*/    
	mrecv_sock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);	
	addr.sin_port=htons(mult_port);
	int on=1;
       setsockopt(mrecv_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(bind(mrecv_sock, (struct sockaddr*) &addr, sizeof(addr))==-1){
		printf("bind() error");
		close(mrecv_sock);
		exit(1);	
	}
	join_addr.imr_multiaddr.s_addr=inet_addr(mult_ip);
	join_addr.imr_interface.s_addr=htonl(INADDR_ANY);
	if ( (setsockopt(mrecv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_addr, sizeof(join_addr)))< 0 ){
		printf(" SetsockOpt Join Error \n");
		close(mrecv_sock);
		exit(1);
	}
  /*------------완료--------------*/
 
  
	char status[1] ="1";
	char check[2];
	char port[4] ={0};
	char name[300];
		/*멀티케스트 메세지 입력 받아서 연결 여부 묻고 접속 코드*/
		memset(buf, 0, BUF_SIZE);
		memset(port, 0, BUF_SIZE);
		str_len=recvfrom(mrecv_sock, buf, BUF_SIZE, 0, NULL, 0);//buf 는 아이피
		str_len=recvfrom(mrecv_sock, port, BUF_SIZE, 0, NULL, 0);//port는 포트
		printf("호스트로 부터 IP 주소를 받았습니다. 접속 하시겠습니까? Y or N\n");
	    fgets(check,2,stdin);//접속 승인 여부
		
		
	if(strcmp(check,"y") ==0||strcmp(check, "Y")==0){
		
		
	int sock;
	char message[BUF_SIZE]={0, };
	struct sockaddr_in serv_adr;
	 int servLen;
	sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(sock==-1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(buf);
	serv_adr.sin_port=htons(atoi(port));
		
	struct timeval tv;//시간 변수
		fd_set readfds;//시간읽기
		int ret; //대기조건
		char ipstr[20]={0};
		char personal_number[2];
		char rt[100];
		GetMyIpAddr(ipstr);
		 sendto(sock, "@ip",10, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 sendto(sock, ipstr, 20, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 printf("이름을 입력해주시길 바랍니다.(특수문자: @제외)\n");
		 fgets(name,2,stdin);//접속 승인 여부
		 fgets(name,1000,stdin);
		 sendto(sock, "@name", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));	    
		 sendto(sock, name, 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 printf("접속을 시작합니다\n");
		  servLen = sizeof(serv_adr);
		 recvfrom(sock,personal_number,10,0,(struct sockaddr *)&serv_adr,&servLen);
		printf("접속이 완료되었습니다\n");
		printf("\n*---------------*\n");
        printf("명령어: @show_clients(다른 사용자 상태값 호출) \n");
        printf("명령어: @connect (상대 IP) (IP를 통해 호스트 연결) \n");
		printf("\n명령어를 입력하고 싶을 시 아무키나 누르시오:\n");
	    printf("*---------------*\n");
	   //pthread_t thread;
       
	   //pthread_create(&thread,NULL,control,strcat(port,buf));
	   
	   while(1){
		   
	   /* 반복해서 서버에게 자신의 상태값 메세지로 보냄*/
   	   // 표준 입력에서 입력 대기
       FD_ZERO(&readfds);
       FD_SET(STDIN_FILENO, &readfds);

       // select가 5초 동안 기다리도록 timeval 구조체 설정
       tv.tv_sec = TIMEOUT;
       tv.tv_usec = 0;
       // select() 시스템콜을 이용해 입력 대기
       ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);  
       if (ret == -1) perror("select");
       else if (!ret){
		   memset(&rt, 0, sizeof(rt));
		   strcpy(rt,personal_number);
		   strcat(rt,status);
		   
		   sendto(sock, "@heartbeat", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		   sendto(sock, rt, 2, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
	   }
       /*--------------------------------*/	
	  
	  if(_kbhit()){
		 char command[40];
		 char msg[100];
		 fgets(command,1000,stdin);
		 if(strcmp(command,"@show_clients")==0){
			  sendto(sock, "@show_clients", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
			  while(strcmp(msg,"-------")!=0){
				  recvfrom(sock,msg,10,0,(struct sockaddr *)&serv_adr,&servLen);
				  fputs(msg,stdout);
			  }
		 }
		}
	   
	  }
	   close(sock);
	}
		
		
		else if(strcmp(check ,"N")==0|| strcmp(check, "n")==0)printf("프로그램을 종료합니다.\n");
		else printf("미확인 명령어입니다. 다시 시작하십시오.\n");
		/*------------미완성-------------*/
	 

	/*마지막으로 모든 소켓들을 종료한다.*/
	close(mrecv_sock);
	
	return 0;
}

void error_handling(char * message){
 fputs(message, stderr);
 fputc('\n',stderr);
 exit(1);
}


/*void * single_chatting(){
	 int single_port =2400;
    int sock,recv_sock;
	char smessage[BUF_SIZE];
	char rmessage[BUF_SIZE];
	int str_len2;
	
	struct sockaddr_in serv_adr, from_adr;
	struct sockaddr_in addr;
	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==-1)
		error_handling("socket() error");

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr("172.32.32.32");
	serv_adr.sin_port=htons(single_port);
    
	recv_sock=socket(PF_INET, SOCK_DGRAM, 0);
 	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);	
	addr.sin_port=htons(single_port);
    
           fputs("랜덤 채팅에 입장하십니다.\n", stdout);
		   while(1){
		   fputs("q 누를 시 나가집니다.\n", stdout);
		   fgets(rmessage, sizeof(rmessage), stdin);     
	       if(!strcmp(rmessage,"q\n") || !strcmp(rmessage,"Q\n")) break;
	       sendto(sock, smessage, strlen(smessage), 0, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
           str_len2=recvfrom(recv_sock, rmessage, BUF_SIZE, 0, NULL, 0);	 
           fputs(rmessage,stdout);	   
		   }
}

	   }*/