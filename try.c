//huge prop to dbus-specification
//https://dbus.freedesktop.org/doc/dbus-specification.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>

#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/uio.h>

#define log_error(is_ok,what_error,error_msg) \
     if(is_ok<0){ \
        fprintf(stderr,what_error,error_msg); \
        return -1; \
     }

#define log_error_exit(is_ok,what_error,error_msg) \
     if(is_ok<0){ \
        fprintf(stderr,what_error,error_msg); \
        exit(-1); \
     }

void get_song_name(struct iovec* msg_iov1,struct iovec* msg_iov2,struct iovec* msg_iov_arr2,struct msghdr* msg_hdr,int sockfd,struct msghdr* recv_msg,ssize_t* received,unsigned char* recv_buf){
      msg_iov1->iov_base="l\1\0\0011\0\0\0\7\0\0\0\210\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\37\0\0\0org.freedesktop.DBus.Properties\0\3\1s\0\3\0\0\0Get\0\0\0\0\0\10\1g\0\2ss\0";
   msg_iov1->iov_len=152;
   msg_iov2->iov_base="\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\10\0\0\0Metadata\0";
   msg_iov2->iov_len=49;
   msg_iov_arr2[0]=*msg_iov1;
   msg_iov_arr2[1]=*msg_iov2;
   msg_hdr->msg_iovlen=2;
   msg_hdr->msg_controllen=0;
   msg_hdr->msg_flags=0;
   int is_ok;
   is_ok=sendmsg(sockfd,msg_hdr,MSG_NOSIGNAL);
   log_error_exit(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN},1,25000);
   log_error_exit(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   *received=recvmsg(sockfd,recv_msg, MSG_CMSG_CLOEXEC);
   log_error_exit(*received,"RECVMSG:ERROR:%s\n",strerror(errno));

   setlocale(LC_ALL,"");
   //fwrite(recv_buf, 1, *received, stdout);
   //putchar('\n');

   unsigned char* buf=recv_buf;
   ssize_t len=*received;

   const char* keys[]={
      "xesam:title",
      "xesam:artist",
      "xesam:album",
      "mpris:length",
      NULL
   };

   printf("Now playing\n");

    for(int k=0;keys[k]!=NULL;k++){
       const char *key = keys[k];
       size_t key_len = strlen(key);
        
       for(ssize_t i=0; i<=len-(ssize_t)key_len;i++){
          if(memcmp(&buf[i], key, key_len) == 0){
             ssize_t pos=i+key_len;
             while (pos < len && buf[pos] == 0) pos++;
                
             if(pos<len&&buf[pos]>=1&&buf[pos]<=10){
                unsigned char sig_len=buf[pos];
                if(pos+1+sig_len<len&&buf[pos+1+sig_len]==0){
                   pos+=1+sig_len+1;
                }
             }
                
             while(pos<len&&(buf[pos]<0x20||buf[pos]==0x7F)) pos++;
                
             if(pos<len){
                ssize_t start=pos;
                while(pos<len){
                        unsigned char c=buf[pos];
                        if(c>=0x20&&c<0x7F){
                           pos++;
                        }else if(c>=0xC0){
                           int seq_len=0;
                           if(c>=0xF0) seq_len=4;
                           else if (c>=0xE0) seq_len=3;
                           else if (c>=0xC0) seq_len=2;
                            
                          int valid=1;
                          for(int j=1;j<seq_len&&pos+j<len;j++){
                             if((buf[pos+j]&0xC0)!=0x80){
                                valid=0;
                                break;
                             }
                          }
                          if(valid) pos+=seq_len;
                          else break;
                        }else{
                           break;
                        }
                }
                    
                if(pos>start){
                   printf("  %s: ", key);
                   fwrite(&buf[start], 1, pos - start, stdout);
                  printf("\n");
                }
             }
                break;
            }
        }
    }
}

void play_song(struct iovec* msg_iov1,struct iovec* msg_iov2,struct iovec* msg_iov_arr2,struct msghdr* msg_hdr,int sockfd){
   msg_iov1->iov_base="l\1\0\1\0\0\0\0\5\0\0\0}\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\3\1s\0\4\0\0\0Play\0\0\0\0";
   msg_iov1->iov_len=144;
   msg_iov2->iov_base="";
   msg_iov2->iov_len=0;
   msg_iov_arr2[0]=*msg_iov1;
   msg_iov_arr2[1]=*msg_iov2;
   msg_hdr->msg_iovlen=2;
   msg_hdr->msg_controllen=0;
   msg_hdr->msg_flags=0;
   int is_ok;
   is_ok=sendmsg(sockfd,msg_hdr,MSG_NOSIGNAL);
   log_error_exit(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   printf(">play song\n");

}

void pause_song(struct iovec* msg_iov1,struct iovec* msg_iov2,struct iovec* msg_iov_arr2,struct msghdr* msg_hdr,int sockfd){
   msg_iov1->iov_base="l\1\0\1\0\0\0\0\5\0\0\0~\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\3\1s\0\5\0\0\0Pause\0\0\0";
   msg_iov1->iov_len=144;
   msg_iov2->iov_base="";
   msg_iov2->iov_len=0;
   msg_iov_arr2[0]=*msg_iov1;
   msg_iov_arr2[1]=*msg_iov2;
   msg_hdr->msg_iovlen=2;
   msg_hdr->msg_controllen=0;
   msg_hdr->msg_flags=0;
   int is_ok;
   is_ok=sendmsg(sockfd,msg_hdr,MSG_NOSIGNAL);
   log_error_exit(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   printf(">pause song\n");
}

void next_song(struct iovec* msg_iov1,struct iovec* msg_iov2,struct iovec* msg_iov_arr2,struct msghdr* msg_hdr,int sockfd){
   msg_iov1->iov_base="l\1\0\1\0\0\0\0\5\0\0\0}\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\3\1s\0\4\0\0\0Next\0\0\0\0";
   msg_iov1->iov_len=144;
   msg_iov2->iov_base="";
   msg_iov2->iov_len=0;
   msg_iov_arr2[0]=*msg_iov1;
   msg_iov_arr2[1]=*msg_iov2;
   msg_hdr->msg_iovlen=2;
   msg_hdr->msg_controllen=0;
   msg_hdr->msg_flags=0;
   int is_ok;
   is_ok=sendmsg(sockfd,msg_hdr,MSG_NOSIGNAL);
   log_error_exit(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   printf(">next song\n");

}

void previous_song(struct iovec* msg_iov1,struct iovec* msg_iov2,struct iovec* msg_iov_arr2,struct msghdr* msg_hdr,int sockfd){
   msg_iov1->iov_base="l\1\0\1\0\0\0\0\5\0\0\0\201\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\3\1s\0\10\0\0\0Previous\0\0\0\0\0\0\0\0";
   msg_iov1->iov_len=152;
   msg_iov2->iov_base="";
   msg_iov2->iov_len=0;
   msg_iov_arr2[0]=*msg_iov1;
   msg_iov_arr2[1]=*msg_iov2;
   msg_hdr->msg_iovlen=2;
   msg_hdr->msg_controllen=0;
   msg_hdr->msg_flags=0;
   int is_ok;
   is_ok=sendmsg(sockfd,msg_hdr,MSG_NOSIGNAL);
   log_error_exit(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   printf(">previous song\n");

}

void display_help(){
        printf("dbus spotify controller\n");
        printf(">press p to play song\n");
        printf(">press s to pause song\n");
        printf(">press i to show song info\n");
        printf(">press b to go back to previous song\n");
        printf(">press n to go to next song\n");
        printf(">press h to print help message\n");
        printf(">press q to quit the application\n");

}

int main(){
   int sockfd=socket(AF_UNIX,SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK,0);
   struct sockaddr_un un_addr;
   memset(&un_addr,0,sizeof(un_addr));
   un_addr.sun_family=AF_UNIX;
   strncpy(un_addr.sun_path,"/run/user/1000/bus",sizeof(un_addr.sun_path)-1);

   int is_ok=connect(sockfd,(struct sockaddr*)&un_addr,sizeof(un_addr));
   log_error(is_ok,"CONNECT::ERROR:%s\n",strerror(errno));

   struct sockaddr_in my_addr;
   socklen_t len;

   is_ok=getsockname(sockfd,(struct sockaddr *)&my_addr,&len);
   log_error(is_ok,"GETSOCKNAME::ERROR:%s\n",strerror(errno));

   is_ok=sendto(sockfd,"\0",1,MSG_NOSIGNAL,NULL,0);
   log_error(is_ok,"SENDTO::ERROR:%s\n",strerror(errno));

   //https://dbus.freedesktop.org/doc/dbus-specification.html#auth-protocol
   //Before the flow of messages begins, two applications must authenticate
   //A simple plain-text protocol is used for authentication
   //The message encoding is NOT used here, only plain text messages.

   //The AUTH command is sent by the client to the server. 
   //The server replies with DATA, OK or REJECTED. 
   is_ok=sendto(sockfd,"AUTH EXTERNAL 31303030\r\n",24,MSG_NOSIGNAL,NULL,0);
   log_error(is_ok,"SENDTO::ERROR:%s\n",strerror(errno));

   //POLLIN There is data to read.
   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN},1,-1);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   //read shit
   char buff[2048];
   is_ok=read(sockfd,buff,2048);
   log_error(is_ok,"READ::ERROR:%s\n",strerror(errno));

   //printf("%s\n",buff);

   //POLLOUT Writing is now possible
   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLOUT},1,-1);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   //The NEGOTIATE_UNIX_FD command is sent by the client to the server. 
   //The server replies with AGREE_UNIX_FD or ERROR
   is_ok=sendto(sockfd,"NEGOTIATE_UNIX_FD\r\n",19,MSG_NOSIGNAL,NULL,0);
   log_error(is_ok,"SENDTO::ERROR:%s\n",strerror(errno));

   //POLLIN There is data to read.
   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN},1,-1);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   //read shit from server shoulod get AGREE_UNIX_FD not ERROR
   is_ok=read(sockfd,buff,2048);
   //printf("%s\n",buff);

   //there are shit to write
   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLOUT},1,-1);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   //The BEGIN command is sent by the client to the server. The server does not reply.
   //The BEGIN command acknowledges that the client has received an OK command from the server
   //and completed any feature negotiation that it wishes to do,
   //and declares that the stream of messages is about to begin.
   is_ok=sendto(sockfd,"BEGIN\r\n",7,MSG_NOSIGNAL,NULL,0);
   log_error(is_ok,"SENDTO::ERROR:%s\n",strerror(errno));

   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN|POLLOUT},1,-1);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

/*
           struct msghdr {
               void         *msg_name;       // Optional address 
               socklen_t     msg_namelen;    // Size of address 
               struct iovec *msg_iov;        // Scatter/gather array 
               size_t        msg_iovlen;     // # elements in msg_iov 
               void         *msg_control;    // Ancillary data, see below 
               size_t        msg_controllen; // Ancillary data buffer size 
               int           msg_flags;      // Flags (unused) 
           };
*/
   char* msg_name=NULL;
   socklen_t msg_namelen=0;
   struct iovec msg_iov1;
   msg_iov1.iov_base="l\1\0\1\0\0\0\0\1\0\0\0n\0\0\0\1\1o\0\25\0\0\0/org/freedesktop/DBus\0\0\0\6\1s\0\24\0\0\0org.freedesktop.DBus\0\0\0\0\2\1s\0\24\0\0\0org.freedesktop.DBus\0\0\0\0\3\1s\0\5\0\0\0Hello\0\0\0";
   msg_iov1.iov_len=128;
   struct iovec msg_iov2;
   msg_iov2.iov_base="";
   msg_iov2.iov_len=0;
   struct iovec msg_iov_arr2[2]={msg_iov1,msg_iov2};
   size_t msg_iovlen=2;
   //msg_control=0
   size_t msg_controllen=0;
   int msg_flags=0;
   int send_msg_flag=MSG_NOSIGNAL;
   struct msghdr msg_hdr;
   msg_hdr.msg_name=msg_name;
   msg_hdr.msg_namelen=msg_namelen;
   msg_hdr.msg_iov=msg_iov_arr2;
   msg_hdr.msg_iovlen=msg_iovlen;
   msg_hdr.msg_control=0;
   msg_hdr.msg_controllen=msg_controllen;
   msg_hdr.msg_flags=msg_flags;

   //sendmsg(int sockfd, const struct msghdr *msg, int flags);
   is_ok=sendmsg(sockfd,(struct msghdr*)&msg_hdr,MSG_NOSIGNAL);
   log_error(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));
 
   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN},1,25000);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno))

   //246
   unsigned long recv_buf_size=1000*100;
   unsigned char* recv_buf=malloc(recv_buf_size*sizeof(unsigned char));
   //man 3 iovec
   struct iovec recv_iov={
      .iov_base=recv_buf,
      .iov_len=recv_buf_size*sizeof(unsigned char)
   };

   struct msghdr recv_msg={
      .msg_iov=&recv_iov,
      .msg_iovlen=1
   };

   // ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
   //is_ok=recvmsg(sockfd,&recv_msg, MSG_CMSG_CLOEXEC);
   ssize_t received=recvmsg(sockfd,&recv_msg, MSG_CMSG_CLOEXEC);
   log_error(received,"RECVMSG:ERROR:%s\n",strerror(errno));

   /*
   printf("recieved message\n");
   for(ssize_t i=0;i<received;i++){
     if(isprint(recv_buf[i])){
        putchar(recv_buf[i]);
     }
     else{
        printf("%x",recv_buf[i]);
     }
   }
   putchar('\n');
   */

   /*
   msg_iov1.iov_base="l\1\0\0011\0\0\0\7\0\0\0\210\0\0\0\1\1o\0\27\0\0\0/org/mpris/MediaPlayer2\0\6\1s\0\36\0\0\0org.mpris.MediaPlayer2.spotify\0\0\2\1s\0\37\0\0\0org.freedesktop.DBus.Properties\0\3\1s\0\3\0\0\0Get\0\0\0\0\0\10\1g\0\2ss\0";
   msg_iov1.iov_len=152;
   msg_iov2.iov_base="\35\0\0\0org.mpris.MediaPlayer2.Player\0\0\0\10\0\0\0Metadata\0";
   msg_iov2.iov_len=49;
   msg_iov_arr2[0]=msg_iov1;
   msg_iov_arr2[1]=msg_iov2;
   msg_hdr.msg_iovlen=2;
   msg_hdr.msg_controllen=0;
   msg_hdr.msg_flags=0;
   is_ok=sendmsg(sockfd,(struct msghdr*)&msg_hdr,MSG_NOSIGNAL);
   log_error(is_ok,"SENDMSG::ERROR:%s\n",strerror(errno));

   is_ok=poll(&(struct pollfd){.fd=sockfd,.events=POLLIN},1,25000);
   log_error(is_ok,"POLL::ERROR:%s\n",strerror(errno));

   received=recvmsg(sockfd,&recv_msg, MSG_CMSG_CLOEXEC);
   log_error(received,"RECVMSG:ERROR:%s\n",strerror(errno));

   printf("recieved message\n");
   setlocale(LC_ALL,"");
   fwrite(recv_buf, 1, received, stdout);
   putchar('\n');
   */
   printf("dbus spotify controller\n");
   printf("dbus spotify controller\n");
   printf(">press p to play song\n");
   printf(">press s to pause song\n");
   printf(">press i to show song info\n");
   printf(">press b to go back to previous song\n");
   printf(">press n to go to next song\n");
   printf(">press h to print help message\n");
   printf(">press q to quit the application\n");

   char opt='r';
   while(opt!='q'){
     opt=fgetc(stdin);
     if(opt=='h'){
	display_help();
     }
     else if(opt=='p'){
        play_song(&msg_iov1,&msg_iov2,msg_iov_arr2,&msg_hdr,sockfd);
     }
     else if(opt=='s'){
        pause_song(&msg_iov1,&msg_iov2,msg_iov_arr2,&msg_hdr,sockfd);
     }
     else if(opt=='i'){
        get_song_name(&msg_iov1,&msg_iov2,msg_iov_arr2,&msg_hdr,sockfd,&recv_msg,&received,recv_buf);
     }
     else if(opt=='b'){
        previous_song(&msg_iov1,&msg_iov2,msg_iov_arr2,&msg_hdr,sockfd);
     }
     else if(opt=='n'){
        next_song(&msg_iov1,&msg_iov2,msg_iov_arr2,&msg_hdr,sockfd);
     }
     else{
         display_help();
     }
   }


   free(recv_buf);
   return 0;
}
