#ifndef __GETCFG_H_
#define __GETCFG_H_


extern char ipaddr[16];//���ڴ�Ż�ȡ��Ip��ַ
extern char dns_addr[50];//���ڴ�Ŵ��Զ��������ַ
extern int  GetConfigValue( char* FileName,char *RS232,int BAD[],int IPPORT[]);
extern void serialdata_handle(char buff[],int len);
extern void handle_sigchld(int sig);

#endif