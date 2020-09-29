/*Libraries Imported*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

/*Packet Structure*/
typedef struct packet{
	char data[484];	
}Packet;


/*Frame Structure*/
typedef struct frame{
	int frame_kind;
	int sq_no;
	int ack;
	int recvdata;
	Packet packet;	
}Frame;


/*Program main Function*/
void main(int argc, char **argv){

	if(argc != 3){
		printf("Usage: %s < Sender-filename> <port>\n", argv[0]);
		exit(0);
	}
	/*Timeout structure*/
	struct timeval timeVal;
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 3000000;

	/*Creating Socket*/
	int port = atoi(argv[2]);
	int sockfd;
	struct sockaddr_in serverAddr, newAddr;
	char buffer[484];
	socklen_t addr_size;


	Frame frame_send; 
	Frame frame_recv;

	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");



	/*Setting the time behaviour of our socket*/
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeVal,sizeof(timeVal));

	/*Opening File to Send*/
        FILE *fp = fopen(argv[1],"rb");
        if(fp==NULL)
        {
            printf("File opern error");
            
        }

	/*Defining Variables*/
	char packet_data[5][484]; 
	int nread = 0;
	int frame_id[5] = {0,1,2,3,4};
	int frame_resend_id[5] = {0};
	int sending =1;
	int resend = 0;
	int ackData[5] = {0};
	int temp = 0;
	

	/*Main Function to receive the file*/
	while(sending == 1){
		
		int read = 0;
		int addr_size = sizeof(serverAddr);
		if(resend == 0){
			for(int i = 0; i < 5 ; i++){
				
				unsigned char buffer[484]={0};
				nread = fread(buffer,1,sizeof(buffer),fp);
				memcpy(frame_send.packet.data,buffer,nread);
				memcpy(packet_data[i],buffer,nread);

				frame_send.frame_kind = 1;
				frame_send.sq_no = frame_id[i]; 
				frame_send.ack = 0;
				frame_send.recvdata = nread;
				
				read++;
				printf("[+]Frame Sent-%i\n",i);

				if(nread > 0){
					sendto(sockfd, &frame_send, sizeof(Frame), 0,(struct sockaddr*)&serverAddr, sizeof(serverAddr));
				}

				if (nread < 484){

					if (feof(fp))
					    printf("End of file\n");
					if (ferror(fp))
					    printf("Error reading\n");
					sending = 0;
					break;
				}
			
			}

				
			for(int i = 0 ; i < read ; i++){
				
				int recv = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0,(struct sockaddr*)&serverAddr, &addr_size);
				
				if(recv >=0 && frame_recv.frame_kind == 0 && frame_recv.ack == frame_id[frame_recv.sq_no]+1){
					printf("[+]Ack Received-%i\n",frame_recv.sq_no);
					ackData[frame_recv.sq_no] = 1;
				}
			}
		
		}
		else{
			printf("Resending.\n");
			for(int i = 0; i < resend ; i++){
				
				unsigned char buffer[484]={0};
				
				memcpy(frame_send.packet.data,packet_data[frame_resend_id[i]],484);

				frame_send.frame_kind = 1;
				frame_send.sq_no = frame_resend_id[i]; 
				frame_send.ack = 0;
				frame_send.recvdata = 484;
				
				printf("[+]Frame re-Sent-%i\n",frame_send.sq_no);

				sendto(sockfd, &frame_send, sizeof(Frame), 0,(struct sockaddr*)&serverAddr, sizeof(serverAddr));
			
			}

				
			for(int i = 0 ; i < resend ; i++){
				
				int recv = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0,(struct sockaddr*)&serverAddr, &addr_size);
				
				if(recv >=0 && frame_recv.frame_kind == 0 && frame_recv.ack == frame_resend_id[i]+1){
					printf("[+]Ack re-Received-%i\n",frame_recv.sq_no);
					ackData[frame_resend_id[i]] = 1;
				}
			}
			
		}
		resend = 0;
		for(int i=0;i<5;i++){
			if(ackData[i] == 0){
				 frame_resend_id[i] = i;
				 resend++;
			}
		}
		
		for(int i=0;i<5;i++){
			for(int j=i+1;j<5;j++){
				if(frame_resend_id[i]<frame_resend_id[j]){
					temp = frame_resend_id[i];
					frame_resend_id[i] = frame_resend_id[j];
					frame_resend_id[j] = temp;
				}
			}
		}
		for(int i=0;i<resend;i++){
			for(int j=i+1;j<resend;j++){
				if(frame_resend_id[i]>frame_resend_id[j]){
					temp = frame_resend_id[i];
					frame_resend_id[i] = frame_resend_id[j];
					frame_resend_id[j] = temp;
				}
			}
		}
		
		if(resend == 0){
			for(int i=0;i<5;i++){
				ackData[i] = 0;
				frame_resend_id[i] = 0;
			}
		}
	}
	/*Closing the file pointer and Socket*/
	fclose(fp);
	close(sockfd);
}