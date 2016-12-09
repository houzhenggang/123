#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <termios.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <pthread.h>
#include<stdbool.h>
#include<signal.h>
#include<sys/wait.h>
#include "serial.h"
#include "getcfg.h"
#include "list.h"


char cliend_buf[128];//�������ݻ������SOCKET����
int sockread_len;//�ͻ��˶�ȡ���ֽ���

char ipaddr[16];//���ڴ�Ż�ȡ��Ip��ַ
char dns_addr[50];//���ڴ�Ŵ��Զ��������ַ
int ip_port_num[2];//���ڴ�ű��غ�Զ�̶˿�
  
char RS232_BUF[2];//���ڶ˿�
int RS232_BAUD[2];//���ڲ�����


//INFODATA info_data;

int serial_fd[2];


/*struct Infodata {
	int socfd;
	char * string;
	}INFODATA;
*/
/*function prototype*/

void setnonblocking(int sock)
{
	int opts;
	opts=fcntl(sock,F_GETFL);
	if(opts<0)
	{
		perror("fcntl GETFL");
		exit(1);
	}
	opts=opts|O_NONBLOCK;
	if(fcntl(sock,F_SETFL,opts)<0)
	{
		perror("fcntl SETFL");
		exit(1);
	}
}

void* ClientThread(void *recv_id)
{
	int rev_fd = *(int *)recv_id;
	int j;	
	/* ���տͻ��˷��������ݲ���ʾ���� */
		
		if (sockread_len <= 0)
		{
			close(rev_fd);
			pthread_exit(NULL);
		}
		else
		{
			//д��ѡ�еĴ�����
			//write(*recv_id,buf,ret);
			for (j=0;j<2;j++)
			{
			if(serial_fd[j]>2)	
				{
					write(serial_fd[j],cliend_buf,sockread_len);
					printf("rcv socket:%s\n",cliend_buf);
				}
			else
				continue;
			}
		}	

}

int main()
{
  pthread_t  Client_ID;
  //�����߳�
  signal(SIGCHLD,handle_sigchld);
  createNodelist();  //�����������ڴ洢FD
  //���ڳ�ʼ����ȡ·��ֵ���ô����Լ�Զ�����Ӷ˿�
  //GetConfigValue("config.txt" , RS232_BUF,RS232_BAUD, ip_port_num);
  GetConfigValue("/etc/config/rs232" , RS232_BUF,RS232_BAUD, ip_port_num);
  
  printf("client ipport is is %d\n",ip_port_num[0]);//���ض˿�
  printf("server ipport is is %d\n",ip_port_num[1]);//Զ�̶˿�
  printf("the server ip is%s\n",ipaddr);
   printf("the server dns is%s\n",dns_addr);
  
   /*����epoll_event�ṹ��ı�����ev����ע���¼���events
   �������ڻش�Ҫ������¼�*/
   
   struct epoll_event ev,events[20];//���������ڻش����յ��¼�
   /*�������ڴ���accept��epollר��
   ���ļ���������ָ�����������������ΧΪ256*/
  int epoll_instance=epoll_create(200);//size is unused nowadays

  int  listenfd = socket(AF_INET,SOCK_STREAM,0);
   if(listenfd <0)
   {
      perror("error opening socket");
      return -1;
   }
   //setnonblocking(listenfd);//�����ڼ�����socket���óɷ�������ʽ
   ev.data.fd=listenfd;//������Ҫ������¼���ص��ļ�������
   ev.events=EPOLLIN|EPOLLET;//����Ҫ������¼�����
   epoll_ctl(epoll_instance,EPOLL_CTL_ADD,listenfd,&ev);//ע��epoll�¼�
/*
���ط��������ã��󶨶˿ڣ�����ǿͻ���ֱ�Ӷ�ȡԶ�̵�ip
*/
   struct sockaddr_in servaddr;
   memset(&servaddr,0,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = INADDR_ANY;
   servaddr.sin_port = htons((uint16_t )ip_port_num[0]);
/*����˿ڸ�������*/
int opt = 1;
int sizeInt_len = sizeof(opt);
  if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(char *)&opt,sizeInt_len)==-1)
  	{       
  	perror("setsockopt");       
	return -1;    
	}
  
  int ret = bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
   if(ret <0)
   {
      perror("Error on binding");
      return -1;
   } 
   ret = listen(listenfd,5);//backlog
   if(ret !=0)
   {
       perror("Error on listening");
       return -1;
   }

/*����Զ������
*/
  int  ser_lister = socket(AF_INET,SOCK_STREAM,0);
  if(ser_lister<0)
  	{
  		perror("error opening server_socket\n");
     		return -1;
  	}
  /*��Զ����������epoll �¼�����*/
   ev.data.fd=ser_lister ;//������Ҫ������¼���ص��ļ�������
   ev.events=EPOLLIN|EPOLLET;//����Ҫ������¼�����
   epoll_ctl(epoll_instance,EPOLL_CTL_ADD,ser_lister ,&ev);//ע��epoll�¼�
   
   struct sockaddr_in  ser_addr;//Զ�̷�����

   /*���½���SERVER�ڵ����������*/
  Node * sernode=(Node*)malloc(sizeof(Node));
  sernode->fd_data=ser_lister;
  sernode->pNext=NULL;
  addNode(sernode);
  
   memset(&ser_addr,0,sizeof(ser_addr));
   ser_addr.sin_family = AF_INET;
   
  //��������
  if(strcmp(ipaddr,"\0")==0)
  	{
  struct hostent *host;
   if((host=gethostbyname(dns_addr))==NULL)
   		{
   		herror("gethostbyname error!\n");
   		}
   ser_addr.sin_addr =*((struct in_addr *)host->h_addr);
  	}
  else
  	{
  // char *addr_string = "192.168.1.50";
   //ser_addr.sin_addr.s_addr = inet_addr(addr_string);
   //inet_pton(AF_INET,ip_addr,&ser_addr.sin_addr);  
   //ser_addr.sin_addr.s_addr = inet_addr(string_ser_addr);
   //����IP
 if( inet_aton(ipaddr, &ser_addr.sin_addr)==0)
  	{
  	perror("connect server erro!\n");
	return -1;
	 }
  	}
  //���ö˿�
   ser_addr.sin_port = htons((uint16_t )ip_port_num[1]);
   
   /*���ö˿ڸ���*/
   int opt_1= 1;
   int sizeInt_len_1 = sizeof(opt);
   if(setsockopt(ser_lister,SOL_SOCKET,SO_REUSEADDR,(char *)&opt_1,sizeInt_len_1)==-1)
  	{       
  	perror("setsockopt\n");      
	return -1;    
	}
    if(connect(ser_lister,(struct sockaddr*)&ser_addr,sizeof(ser_addr))<0)
    	{
    	 perror("connect serverip erro!\n");       
	return -1;   
    	}

//�����豸�ļ�
   int file_num;
   for (file_num= 0;file_num<2;file_num++)
  	{
  	if('1'==*(RS232_BUF+file_num))
  		{
  		 if(( serial_fd[file_num] = open_port(serial_fd[file_num], file_num))<0)
 			{
 			perror("open_port error");
			return;
 			}
		 //���ô���
		 int k;
   		if(( k=set_opt(serial_fd[file_num],RS232_BAUD[file_num],8,'N',1))<0)
			{
			perror("set_opt error\n");
			return;
			}
		if(serial_fd[file_num]>2)
			{
			//�������¼����뵽epoll ��
			ev.data.fd=serial_fd[file_num];//������Ҫ������¼���ص��ļ�������
   			ev.events=EPOLLIN|EPOLLET;//����Ҫ������¼����Ͷ������ش���
   			epoll_ctl(epoll_instance,EPOLL_CTL_ADD,serial_fd[file_num],&ev);//ע��epoll�¼�
			printf("select uart is %d,serial_fd is %d\n",file_num,serial_fd[file_num]);	
			}
  		}
	}

   while(1)
   {
    int nfound=epoll_wait(epoll_instance,events,20,1000);//�ȴ�epoll�¼��ķ���,�����¼���
   	  if(nfound==0)
   	  {
   	  	//printf(".");
   	  	fflush(stdout);
		continue;
   	  }
	  int n_poll;
    for(n_poll=0;n_poll<nfound;n_poll++)
   	  {
   	  	if(events[n_poll].data.fd==listenfd)/*���ض˿�*/
   	  	{
   	  		struct sockaddr_in cliaddr;//Զ�̶˿�
   	  		uint32_t  len = sizeof(cliaddr);
   	  		int connfd=accept(listenfd,(struct sockaddr*)&cliaddr,&len);
   	  		printf("connection from host %s,port %d,sockfd is %d\n",
             		inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port),connfd);
   	  		  //setnonblocking(connfd);
   	  		  
   	  		  ev.data.fd=connfd;//�������ڶ��������ļ�������
   	  		  ev.events=EPOLLIN|EPOLLET;//��������ע��Ķ������¼�
   	  		  epoll_ctl(epoll_instance,EPOLL_CTL_ADD,connfd,&ev);//ע��ev�¼�
			  printf("add %d to epoll\n",connfd);
			  
			//info_data.socfd=connfd;
			//printf("connect is %d\n,info_data.socfd");
			//���ϵͳ��Ϣ
			//��ӵ�����
			Node * clientnode=(Node*)malloc(sizeof(Node));
			clientnode->fd_data=connfd;
			clientnode->pNext=NULL;
			addNode(clientnode);
   	  	}
		else if(events[n_poll].data.fd==ser_lister)
			{
			char revser_buf[128];
			memset(revser_buf,0,sizeof(revser_buf));
			int reser_len ;
			reser_len = read(ser_lister,revser_buf,sizeof(revser_buf)-1);
			if(reser_len<=0)
				{
   	  		  	  	printf("del server socket\n");
					printf("reconect the server again!\n");
				if(connect(ser_lister,(struct sockaddr*)&ser_addr,sizeof(ser_addr))<0)
    					{
    	 				perror("connect serverip erro!\n");
					 if(deleteNode(ser_lister))
				 		printf("delete the server from list\n");
					return -1;   
    					}
					continue;
				}
			else
				{
					printf("recv %d,from server is %s",reser_len,revser_buf);
					int j;
					for (j=0;j<2;j++)
						{
							if(serial_fd[j]>2)	
								{
									write(serial_fd[j],revser_buf,reser_len);
								}
							else
								continue;
						}
				}
			
			}
   	  	else  if(events[n_poll].events&EPOLLIN)/*���¼�*/
   	  	{    
   	  		int read_cont;//��ȡ�����ڵ��ֽ���
			char serial_buf[128];//���ڴ�Ŵ��ڵ�����
   	  		if(events[n_poll].data.fd==serial_fd[0])
   	  		{
   	  			if((read_cont=read(serial_fd[0],serial_buf,128))>0)
						{
						serialdata_handle(serial_buf,read_cont);
						printf("rev %d from serial is : %s\n",read_cont,serial_buf);
						memset(serial_buf,0,sizeof(serial_buf));
						}
				else
					continue;
   	  		}
		else  if(events[n_poll].data.fd==serial_fd[1])
   	  		{
   	  			if((read_cont=read(serial_fd[1],serial_buf,128))>0)
						{
						serialdata_handle(serial_buf,read_cont);
						printf("rev %d from serial is : %s\n",read_cont,serial_buf);
						memset(serial_buf,0,sizeof(serial_buf));
						}
				else
					continue;
   	  		}	
		//�ж��Ƿ���SOCKET�¼�
		else{
				int sockfd=events[n_poll].data.fd;	
   	  			memset(cliend_buf,0,sizeof(cliend_buf));
				sockread_len=read(sockfd,cliend_buf,sizeof(cliend_buf)-1);
   	  			if(sockread_len<=0)
   	  		 	 {
   	  		   	  	ev.data.fd=sockfd;
   	  		  	  	ev.events=EPOLLIN|EPOLLET;
   	  		   	 	epoll_ctl(epoll_instance,EPOLL_CTL_DEL,sockfd,&ev);
					 if(deleteNode(sockfd))
				 		printf("delete the list\n");
   	  		   	  	close(sockfd);
   	  		  	  	printf("del client\n");
					continue;
   	  		 	 }
				  //�������ݺ����������߳�
				  int *temp=(int *)malloc(sizeof(int));
				  *temp=sockfd;
				  pthread_create(&Client_ID,NULL,&ClientThread,temp);
   	  		 	 //write(sockfd,buf,ret);
			 	 //printf ("rec data is :%s\n",buf);
   	  		 	 //printf("write back to client.....\n");
				}
   	     		}
   		} 
    	}
   pthread_join(&Client_ID,NULL);
   return 0;
}
