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


char status[2] ="1";
	
int _kbhit(int time_out)
{ 
  fd_set rfds;
  struct timeval tv;
  FD_ZERO(&rfds);
  FD_SET(0,&rfds);
  tv.tv_sec = time_out/1000000;
  tv.tv_usec =time_out%1000000;
  
  return select(1,&rfds,NULL,NULL, &tv)>0;
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

void* thread1(){
	
	int serv_sock;
    int clint_sock;
    int len,str_len;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clint_addr;
    socklen_t clnt_addr_size;
     struct timeval tv;
     fd_set readfds,tmpfds;//시간읽기
		int ret; //대기조건
	
		FD_ZERO(&readfds);
       FD_SET(STDIN_FILENO, &readfds);
	   tv.tv_sec = TIMEOUT;
       tv.tv_usec = 0;
	
    serv_sock = socket(PF_INET, SOCK_STREAM,0); //1번
    if(serv_sock == -1)
        printf("socket error\n");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9700);

    if(bind(serv_sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1){ //2번
        printf("bind error\n");
	}
    if(listen(serv_sock,5)==-1) //3번
        printf("listen error\n");
    
    clnt_addr_size = sizeof(clint_addr);
	while(1){
    clint_sock = accept(serv_sock,(struct sockaddr*)&clint_addr,&clnt_addr_size); //4번
    if(clint_sock == -1)
        printf("accept error\n");
    else printf("접속됨\n");
     strcpy(status ,"2");
    char message[100];
	
	printf("[SERV]: Q또는 q를 눌러서 종료\n");
	while(1){
		 tmpfds =readfds;
	   tv.tv_sec = TIMEOUT;
       tv.tv_usec = 0;
       // select() 시스템콜을 이용해 입력 대기
       ret = select(STDIN_FILENO + 1, &tmpfds, NULL, NULL, &tv); 
	   
       if (ret == -1) perror("select");
		
	    else{
			if(FD_ISSET(STDIN_FILENO,&readfds)){
				  char msg[100];
				 len = read(STDIN_FILENO,msg,100);
				 if(len=-1){
					 write(clint_sock, msg, sizeof(msg)-1);
					  if(strcmp(msg,"q\n")==0 ||strcmp(msg,"Q\n")==0)break;
					  memset(&msg, 0, sizeof(msg)-1);
				 }
		}}
		 str_len = read(clint_sock,message,sizeof(message)-1); //3번
			 if(str_len==-1)
				 printf("read error\n");
			 printf("client : %s \n", message);
			  if(strcmp(message,"q\n")==0 ||strcmp(message,"Q\n")==0)break;
			 memset(&message, 0, sizeof(message));
			 
			  
	}
	printf("[SERV]:접속을 종료합니다.\n");
    strcpy(status ,"1");
	close(clint_sock);
	}
  //  close(serv_sock); //6번
//    
	
}

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
	pthread_t thread;
	int new_thread;
	new_thread = pthread_create(&thread,NULL,thread1,NULL);
	if(new_thread<0) printf("쓰레드 에러\n");	
		
	if(strcmp(check,"y") ==0||strcmp(check, "Y")==0){
		
		
	int sock;
	int len,len2;
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
		
	struct timeval tv,tv2;//시간 변수
		fd_set readfds,tmpfds;//시간읽기
		int ret; //대기조건
		FD_ZERO(&readfds);
       FD_SET(STDIN_FILENO, &readfds);
	   tv.tv_sec = TIMEOUT;
       tv.tv_usec = 0;
	   
	   fd_set readfds2,tmpfds2;//시간읽기
	   int ret2; //대기조건
	   FD_ZERO(&readfds2);
       FD_SET(STDIN_FILENO, &readfds2);
	   tv2.tv_sec = TIMEOUT;
       tv2.tv_usec = 0;
	   
		char ipstr[20]={0};
		char personal_number[2];
		char pip[20];
		char rt[100];
		GetMyIpAddr(ipstr);
		 sendto(sock, "@ip",10, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 sendto(sock, ipstr, 20, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 printf("이름을 입력해주시길 바랍니다.(특수문자: @제외)\n");
		 fgets(name,2,stdin);//접속 승인 여부
		 fgets(name,100,stdin);
		 
		(strlen(name) - 1)[name]='\0';
		 sendto(sock, "@name", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));	    
		 sendto(sock, name, 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 printf("접속을 시작합니다\n");
		  servLen = sizeof(serv_adr);
		 recvfrom(sock,personal_number,10,0,(struct sockaddr *)&serv_adr,&servLen);
		printf("접속이 완료되었습니다\n");
		printf("\n*---------------*\n");
        printf("명령어: @show_clients(다른 사용자 상태값 호출) \n");
        printf("명령어: @connect(IP를 통해 호스트 연결) \n");
		printf("\n명령어를 입력하고 싶을 시 명령어를 치시고 엔터키를 누르시오.:\n");
	    printf("*---------------*\n");
	   
	   while(1){
	   
	   tmpfds =readfds;
	   tv.tv_sec = TIMEOUT;
       tv.tv_usec = 0;
       // select() 시스템콜을 이용해 입력 대기
       ret = select(STDIN_FILENO + 1, &tmpfds, NULL, NULL, &tv); 
       if (ret == -1) perror("select");
       else if (ret==0){
		   memset(&rt, 0, sizeof(rt));
		   strcpy(rt,personal_number);
		   strcat(rt,status);
		   
		   sendto(sock, "@heartbeat", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		   sendto(sock, rt, 2, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
	   }
        else{
			if(FD_ISSET(STDIN_FILENO,&readfds)){
		 char command[100];
		 char msg[100];
		 
		 len = read(STDIN_FILENO,command,100);
		 if(len!=-1){
		 if(strcmp (command,"@show_clients\n")==0){
		 sendto(sock, "@show_clients", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		 while(strcmp (msg,"-------")!=0){
		 recvfrom(sock,msg,100,0,(struct sockaddr *)&serv_adr,&servLen);
		 printf("%s\n",msg);
		 fflush(stdout);
		 }
		 }
		 
		 else if(strcmp(command,"@connect\n")==0){
			 strcpy(status,"2");
			 printf("IP입력: ");
			 scanf("%s",pip);
			 rewind(stdin);
			 int my_sock;
			 struct sockaddr_in serv_addr;
			 int str_len;
			 my_sock = socket(PF_INET,SOCK_STREAM,0); //1번
			 if(my_sock == -1) printf("socket error \n");
			 memset(&serv_addr,0,sizeof(serv_addr));
			 serv_addr.sin_family = AF_INET;
			 serv_addr.sin_addr.s_addr=inet_addr(pip);
			 serv_addr.sin_port=htons(9700);
			 if(connect(my_sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))==-1) //2번
			 printf("connect error\n");
			 char message[100];
			  printf("Q또는 q를 눌러서 종료\n");
			 while(1){
		     
			 tmpfds2 =readfds2;
	         tv2.tv_sec = TIMEOUT;
             tv2.tv_usec = 0;
			 ret2 = select(STDIN_FILENO + 1, &tmpfds2, NULL, NULL, &tv2); 
             
			 if (ret2 == -1) perror("select");
             
			 else if (ret2==0){
	 	     memset(&rt, 0, sizeof(rt));
		     strcpy(rt,personal_number);
		     strcat(rt,status);
		     sendto(sock, "@heartbeat", 100, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
		     sendto(sock, rt, 2, 0,(struct sockaddr *)&serv_adr, sizeof(serv_adr));
			 }
			 
			 else{if(FD_ISSET(STDIN_FILENO,&readfds2)){
				  char msg[100];
				 len2 = read(STDIN_FILENO,msg,100);
				 if(len2!=-1){
					 write(my_sock, msg, sizeof(msg)-1);
					 if(strcmp(msg,"q\n")==0 ||strcmp(msg,"Q\n")==0)break;
					  memset(&msg, 0, sizeof(msg)-1);
				 }
			}}
			
			 str_len = read(my_sock,message,sizeof(message)-1); //3번
			 if(str_len==-1)
				 printf("read error2\n");
			 printf("[상대] : %s \n", message);
			 if(strcmp(message,"q\n")==0 ||strcmp(message,"Q\n")==0)break;
			 memset(&message, 0, sizeof(message));
			 
	       }
			
			 close(my_sock); //4번
			 strcpy( status ,"1");
			  printf("접속을 종료합니다.\n");
 		 }
		 
		 else printf("등록되지 않은 명령어 입니다.\n");
		 }
		memset(&command, 0, sizeof(command));	
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

