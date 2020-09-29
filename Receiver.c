/*Libraries Imported*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stdlib.h"
#include <time.h>

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
		printf("Usage: %s <Receiving-filename> <port>\n", argv[0]);
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


	
	Frame frame_recv;
	Frame frame_send;
	
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	addr_size = sizeof(newAddr);

	/*Setting the time behaviour of our socket*/
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeVal,sizeof(timeVal));
	printf(">>>Server is running...\n");

	/*Opening File to Send*/
	FILE *fp;
	fp = fopen(argv[1],"ab");

	/*Defining Variables*/
	int bytesReceived;
	int frame_id[5] = {0,1,2,3,4};
	int sending = 1;
	int write = 0;
	char packet_data[5][484];
	int rereceive =0;
	int frame_rereceive_id[5]={0};
	int temp = 0;
	int ackData[5] = {0};
	
	/*Main Function to receive the file*/
	while(sending == 1){
		
		if(rereceive==0){
			for(int i=0;i<5;i++){
				
				bytesReceived = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&newAddr, &addr_size);
				
				if(frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id[frame_recv.sq_no]){
					
					printf("[+]Frame received %d\n",frame_recv.sq_no);
					printf("Data received-%i\n",bytesReceived);

					char buffer[frame_recv.recvdata];
					memset(buffer, '0', sizeof(buffer));
					memcpy(buffer, frame_recv.packet.data,frame_recv.recvdata);
					memcpy(packet_data[frame_recv.sq_no], buffer,frame_recv.recvdata);
					ackData[frame_recv.sq_no] = 1;

					frame_send.frame_kind = 0;
					frame_send.sq_no = frame_id[frame_recv.sq_no];
					frame_send.ack = frame_id[frame_recv.sq_no]+1;
					frame_send.recvdata = 0;
					
					sendto(sockfd, &frame_send, sizeof(Frame), 0,(struct sockaddr*)&newAddr, addr_size);
					
					printf("[+]ACK Sent %i\n",frame_recv.sq_no);

					if(frame_recv.recvdata < 484){
						sending = 0;
						break;
					}
				}
				
			}
		}
		
		else{	
			printf("Rereceiving.\n");
			for(int i=0;i<rereceive;i++){
				bytesReceived = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0, (struct sockaddr*)&newAddr, &addr_size);
				
				if(frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_rereceive_id[i]){
					
					printf("[+]Frame re-received %d\n",frame_rereceive_id[i]);

					char buffer[frame_recv.recvdata];
					memset(buffer, '0', sizeof(buffer));
					memcpy(buffer, frame_recv.packet.data,frame_recv.recvdata);
					memcpy(packet_data[frame_recv.sq_no], buffer,frame_recv.recvdata);
					ackData[frame_rereceive_id[i]] = 1;


					frame_send.frame_kind = 0;
					frame_send.sq_no = frame_rereceive_id[i];
					frame_send.ack = frame_rereceive_id[i]+1;
					frame_send.recvdata = 0;
					
					sendto(sockfd, &frame_send, sizeof(Frame), 0,(struct sockaddr*)&newAddr, addr_size);

					printf("[+]ACK re-Sent %i\n",frame_rereceive_id[i]);

					if(frame_recv.recvdata < 484){
						sending = 0;
						break;
					}
				}
			}
		}
		
		
		rereceive = 0;
		for(int i=0;i<5;i++){
			if(ackData[i] == 0){
				 frame_rereceive_id[i] = i;
				 rereceive++;
			}
		}
		for(int i=0;i<5;i++){
			for(int j=i+1;j<5;j++){
				if(frame_rereceive_id[i]<frame_rereceive_id[j]){
					temp = frame_rereceive_id[i];
					frame_rereceive_id[i] = frame_rereceive_id[j];
					frame_rereceive_id[j] = temp;
				}
			}
		}
		for(int i=0;i<rereceive;i++){
			for(int j=i+1;j<rereceive;j++){
				if(frame_rereceive_id[i]>frame_rereceive_id[j]){
					temp = frame_rereceive_id[i];
					frame_rereceive_id[i] = frame_rereceive_id[j];
					frame_rereceive_id[j] = temp;
				}
			}
		}
		

		if(rereceive == 0 || sending == 0){
			
			for(int i=0;i<5;i++){
				ackData[i] = 0;
				frame_rereceive_id[i] = 0;
			}
			for(int k=0;k<5;k++){
				fwrite(packet_data[k],1,frame_recv.recvdata,fp);
			}
		}
		
	}
	/*Closing the file pointer and Socket*/
	printf(">>>File Received.\n");
	fclose(fp);
	close(sockfd);
}