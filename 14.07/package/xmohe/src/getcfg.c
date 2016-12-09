#include <unistd.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "getcfg.h"
#include "list.h"

//��ȡĿ���ַ����������
int GetValue( char* CFGBuffer, int Buflen, char *pKeyName )
{
	int   i1, i2, len1, len2;
	char pStr[20];

	len1 = strlen( pKeyName );

	for( i1=0; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			break;
		}
	}
	if( i1==Buflen )    return  -1;
	int Flg=0;
	for( i2=0; i1<Buflen; i1++)
	{
		if( (CFGBuffer[i1]==0x27) &&(Flg==0) )//0x27 ��ʾ���ǡ�
		{
			Flg = 1;
			continue;
		}
		if( (CFGBuffer[i1]==0x27)&&(Flg>0) )
			break;
		pStr[i2] = CFGBuffer[i1];
		i2++;
	}
	pStr[i2] = '\0';
	return atoi(pStr);
}

/*��ȡĿ���ߵ��ַ��������������ַ�ָ��*/
void  GetString( char* CFGBuffer, int Buflen, char *pKeyName )
{
	int   i1, i2, len1, len2;
	int   len_temp;
	char pStr[25];

	len1 = strlen( pKeyName );

	for( i1=0; i1<Buflen; i1++ )
	{     //�ַ���ͬ�Ż�0
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			len_temp = i1;
			break;
		}
	}
	if( len_temp==Buflen )   
		{
		memset(ipaddr,0,sizeof(ipaddr));
		return  ;
		}
	if(i1==Buflen)
		{
		memset(ipaddr,0,sizeof(ipaddr));
		return  ;
		}
	int Flg=0;
	for( i2=0; len_temp<Buflen; len_temp++)
	{
		if( (CFGBuffer[len_temp]==0x27) &&(Flg==0) )//0x27 ��ʾ���ǡ�,��ʾ��һ��'
		{
			Flg = 1;
			continue;
		}
		else if( (CFGBuffer[len_temp]==0x27)&&(Flg>0) )
			break;
		else
			{
			pStr[i2] = CFGBuffer[len_temp];
			i2++;
			}
	}
	pStr[i2] = '\0';
	int num_i,num_m;
	for(num_i = 0,num_m=0;num_i < i2;num_i++)
		{
		if(pStr[num_i]!=' ')
			{
		ipaddr[num_m] = pStr[num_i];
		num_m++;
			}
		else
			{
			continue;
			}
		}
	//printf("get the ipaddr is%s\n",get_string);
}


/*��ȡĿ���ߵ��ַ��������������ַ�ָ��*/
void  GetDNS_String( char* CFGBuffer, int Buflen, char *pKeyName )
{
	int   i1, i2, len1, len2;
	int   len_temp;
	char pStr[25];

	len1 = strlen( pKeyName );

	for( i1=0; i1<Buflen; i1++ )
	{     //�ַ���ͬ�Ż�0
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			len_temp = i1;
			break;
		}
	}
	if( len_temp==Buflen )  
		{
		memset(dns_addr,0,sizeof(dns_addr));
		return  ;
		}
	if(i1==Buflen)
		{
		memset(dns_addr,0,sizeof(ipaddr));
		return  ;
		}
	int Flg=0;
	for( i2=0; len_temp<Buflen; len_temp++)
	{
		if( (CFGBuffer[len_temp]==0x27) &&(Flg==0) )//0x27 ��ʾ���ǡ�,��ʾ��һ��'
		{
			Flg = 1;
			continue;
		}
		else if( (CFGBuffer[len_temp]==0x27)&&(Flg>0) )
			break;
		else
			{
			pStr[i2] = CFGBuffer[len_temp];
			i2++;
			}
	}
	pStr[i2] = '\0';
	int num_i,num_m;
	for(num_i = 0,num_m=0;num_i < i2;num_i++)
		{
		if(pStr[num_i]!=' ')
			{
	        dns_addr[num_m] = pStr[num_i];
		num_m++;
			}
		else
			{
			continue;
			}
		}
	//printf("get the ipaddr is%s\n",get_string);
}
/*�ж�pKeyName�����Ƿ�ΪpItemName
*/
int GetCFGValue( char* CFGBuffer, int Buflen, char *pKeyName, char *pItemName )
{
	int   i1, i2, len1, len2,sizelen,bufsize;

	len1 = strlen( pKeyName );
	len2 = strlen( pItemName );

	for( i1=0; i1<Buflen; i1++ )
	{
		if( strncmp( &CFGBuffer[i1], pKeyName, len1 ) == 0 )
		{
			i1 += len1;
			sizelen = i1;
			break;
		}
	}
	if( i1==Buflen )    return  -1;
	int num;
	for(num= sizelen; num<Buflen;num++ )
	{
		if( strncmp( &CFGBuffer[num], pItemName, len2 )== 0 )
		{
			num+= len2;
			bufsize=num;
			break;
		}
	}
	if( (bufsize==Buflen )||(bufsize>(sizelen+10)))   //���ļ�ĩβ���߼������10�ַ�
		{
		sizelen = 0;
		bufsize = 0;
		return -1;
		}
	return 0;
}


int GetConfigValue( char* FileName,char *RS232,int BAD[],int  IPPORT[])
{
	FILE*	fp;
	char  	ValueStr[80];
    	char    	Buffer[1000], str[100];
	char 		rs232_buf[5];
	int		nBytes;
	int		i;
	int		baud[5];
	fp = fopen( FileName, "rt" );
	if( fp!=NULL )
	{
		fread( Buffer, 1000, 1, fp );//����FP
		nBytes = strlen( Buffer );
		fclose( fp );
		if( nBytes > 0 )
		{
		//ע�⴮��0����rs232_1
		if (0 ==  GetCFGValue( Buffer, nBytes, "rs232_1", "'1'")) {
				rs232_buf[0]='1';
				//sprintf(pConfigInfo->ipaddr, "ifconfig eth0 %s", ValueStr);
				baud[0]=GetValue(Buffer,nBytes,"baud_1");
				printf("I get the RS232_1,the baud is %d\n",baud[0]);
				
			} else {
				rs232_buf[0]='0';
				baud[0]=0;
				//strcpy( pConfigInfo->ipaddr , "ifconfig eth0 192.168.1.110");
				printf("the RS232_1 is not ready\n");
			}
			if (0 ==  GetCFGValue( Buffer, nBytes, "rs232_2", "'1'")) {
				rs232_buf[1]='1';
				//sprintf(pConfigInfo->ipaddr, "ifconfig eth0 %s", ValueStr);
				baud[1]=GetValue(Buffer,nBytes,"baud_2");
				printf("I get the RS232_2,the baud is %d\n",baud[1]);
				
			} else {
				rs232_buf[1]='0';
				baud[1]=0;
				//strcpy( pConfigInfo->ipaddr , "ifconfig eth0 192.168.1.110");
				printf("the RS232_2 is not ready\n");
			}
			// get serial parameters
			int n;
			for ( n=0;n<2;n++)
				{
				RS232[n]=rs232_buf[n];
				BAD[n]=baud[n];
				}
			//printf("the buf is %s\n",RS232);
			
			IPPORT[0]=GetValue(Buffer,nBytes,"local_ipport");//��ȡ���ض˿�
			IPPORT[1]=GetValue(Buffer,nBytes,"ser_ipport");//��ȡԶ�̶˿�
			GetString( Buffer,nBytes, "addr_ip" );//��ȡIP��ַ
			GetDNS_String( Buffer,nBytes, "dns_addr" );//��ȡDNS��ַ
		}
	}
	return 0;
}

/*
int iscorrectcfg(CONFIG_INFO* pConfigInfo)
{
	int i;
	
	if (pConfigInfo->TCPBasePort < 1000 || pConfigInfo->TCPBasePort > 20000) 
		pConfigInfo->TCPBasePort = 1234;

	for (i = 0; i < 6; i++) {
		if (pConfigInfo->BaudRate[i] < 300 || pConfigInfo->BaudRate[i] > 230400)
			pConfigInfo->BaudRate[i] = 57600;
		if (pConfigInfo->DataBits[i] < 5 || pConfigInfo->DataBits[i] > 8)
			pConfigInfo->DataBits[i] = 8;
		if (pConfigInfo->Parity[i] != 'N' ||pConfigInfo->Parity[i] != 'n' ||\
				pConfigInfo->Parity[i] != 'O' ||pConfigInfo->Parity[i] != 'o' ||\
				pConfigInfo->Parity[i] != 'E' ||pConfigInfo->Parity[i] != 'e' ||\
				pConfigInfo->Parity[i] != 'S' ||pConfigInfo->Parity[i] != 's' ) {
			pConfigInfo->Parity[i] = 'n';

		}
		if (pConfigInfo->StopBits[i] != 0 || pConfigInfo->StopBits[i] != 1 || pConfigInfo->StopBits[i] != 2)
			 pConfigInfo->StopBits[i] = 1;
		printf("BaudRate[%d] = %d\n", i, pConfigInfo->BaudRate[i]);
		printf("DataBits[%d] = %d\n", i, pConfigInfo->DataBits[i]);
		printf("Parity[%d] = %c\n", i, pConfigInfo->Parity[i]);
		printf("StopBits[%d] = %d\n", i, pConfigInfo->StopBits[i]);
	} 

	return 0;
}
*/

//�������ݴ�����
void serialdata_handle(char buff[],int len)
{
	int send_serial;
	int k;
	if(NULL==head){
		return;
		}
	Node * p = head->pNext;
	while(NULL!=p){
		send_serial = p->fd_data;
		k=write(send_serial,buff,len);
		printf("write %d is %s\n",k,buff);
		p = p->pNext;
		}
	free(p);
	return;
}

void handle_sigchld(int sig)
{
	while(waitpid(-1,NULL,WNOHANG)>0)
		;
}

