#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#include "main.cc"
#include "SBB_util.cc"

//#include "sbb_socket.h"
#define RUN_STRING "RUN"
#define RUN_STRING2 "RUN2"
#define RUN_STRING3 "RUN3"
#define RUN_STRING4 "RUN2,1,0,0"
#define EXIT_STRING "EXIT"

int
main()
{
	/* 
	 * get an internet domain socket 
	 */
	int sd;
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	/* 
	 * set up the socket structure 
	 */
	struct sockaddr_in	sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;

#ifdef SBB_ANY
	/* set to INADDR_ANY if want server to be open to any client on any machine */
	sock_addr.sin_addr.s_addr = INADDR_ANY;
#else
	char	hostname[128];
	/*
	 *  we'll default to this host and call a section 3 func to get this host
	 */
	if( gethostname(hostname,sizeof(hostname)) ){
		fprintf(stderr," SBB gethostname(...) failed errno: %d\n", errno);
		exit(1);
	}
	printf("SBB gethostname() local hostname: \"%s\"\n", hostname);

	/*
	 * set up socket structure for our host machine
	 */
	struct hostent *hp;
	if ((hp = gethostbyname(hostname)) == 0) {
		fprintf(stderr,"SBB gethostbyname(...) failed errno: %d exiting...\n", errno);
		exit(1);
	}
	sock_addr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;
#endif
	sock_addr.sin_port = htons(PORT);

	/* 
	 * bind the socket to the port number 
	 */
	if (bind(sd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
		perror("bind");
		exit(1);
	}

	/* 
	 * advertise we are available on this socket/port
	 */
	while(1){
	if (listen(sd, 5) == -1) {
		perror("listen");
		exit(1);
	}

	/* 
	 * wait for a client to connect 
	 */
	struct sockaddr_in	sock_addr_from_client;
        socklen_t addrlen = sizeof(sock_addr_from_client); 
	int sd_current;
	if ((sd_current = accept(sd, (struct sockaddr *)  &sock_addr_from_client, &addrlen)) == -1) {
		fprintf(stderr,"SBB accept(...) failed errno: %d  exiting...\n", errno);
		exit(1);
	}
	printf("SBB client ip address: %s port: %x\n",
		inet_ntoa(sock_addr_from_client.sin_addr),
		PORT);
	SBB_util cClock; //timer for the msg handler
	cClock.start_clock();
//		ntohs(sock_addr_from_client.sin_port));

	/*
	 * block on socket waiting for client message
	 */
	int ret = 0;
    char	msg[MSGSIZE];
    char	rmsg[MSGSIZE];
    char	elapsedTime[MSGSIZE];
    memset(&msg[0], 0, sizeof(msg));
	fprintf(stderr," SBB: sizeof msg: %lu\n", sizeof(msg));

	
	while ( (ret = recv(sd_current, msg, sizeof(msg), 0) > 0)) {
	
		//START_TIMER(aClock);
			//if client send "RUN" then start timer, run the calculating code, and end timer.
			//if(strcmp(msg,RUN_STRING)==0){
			// 	printf("sbb_server received RUN\n");
			// 	printf("SBB server received msg: [%s] from client\n", msg);
			// 	runcode(rmsg,msg);

			// 	//strcpy(msg," this is the server message response!");
			// 	if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
			// 		fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 		exit(1);
			// 	}
				
			// }

			// if(strcmp(msg,RUN_STRING2)==0){
			// 	printf("sbb_server received RUN2\n");
			// 	printf("SBB server received msg: [%s] from client\n", msg);
			// 	runcode(rmsg,msg);

			// 	//strcpy(msg," this is the server message response!");
			// 	if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
			// 		fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 		exit(1);
			// 	}
				
			// }

			// if(strcmp(msg,RUN_STRING3)==0){
			// 	printf("sbb_server received RUN3\n");
			// 	printf("SBB server received msg: [%s] from client\n", msg);
			// 	runcode(rmsg,msg);

			// 	//strcpy(msg," this is the server message response!");
			// 	if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
			// 		fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 		exit(1);
			// 	}
				
			// }

			// if(strcmp(msg,RUN_STRING4)==0){
			// 	printf("sbb_server received RUN4\n");
			// 	printf("SBB server received msg: [%s] from client\n", msg);
			// 	runcode(rmsg,msg);

			// 	//strcpy(msg," this is the server message response!");
			// 	if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
			// 		fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 		exit(1);
			// 	}
				
			// }

			if (strcmp(msg, EXIT_STRING)==0){
				printf("EXIT SBB server received msg: [%s] from client\n", msg);
				double realt, usert, syst;
				cClock.end_clock(realt, usert, syst);
				sprintf(elapsedTime, "\nREAL seconds: %3.3f - USER space seconds: %3.3f SYSTEM space seconds: %3.3f\n\n",
				realt,
				usert,
				syst);

				if (send(sd_current, elapsedTime, strlen(elapsedTime), 0) == -1) {
				fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
				exit(1);
				}

			}
			else{
				//printf("sbb_server received RUN4\n");
				printf("SBB server received msg: [%s] from client\n", msg);
				runcode(rmsg,msg);

				//strcpy(msg," this is the server message response!");
				if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
					fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
					exit(1);
				}
			}
			// else{
			// 	if (strcmp(msg, EXIT_STRING)==0){
			// 		printf("EXIT SBB server received msg: [%s] from client\n", msg);
			// 		double realt, usert, syst;
			// 		cClock.end_clock(realt, usert, syst);
			// 		sprintf(elapsedTime, "\nREAL seconds: %3.3f - USER space seconds: %3.3f SYSTEM space seconds: %3.3f\n\n",
			// 		realt,
			// 		usert,
			// 		syst);

			// 		if (send(sd_current, elapsedTime, strlen(elapsedTime), 0) == -1) {
			// 		fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 		exit(1);
			// 		}

			// 	}
			// 	/*
			// 	Aby GUI v1, commenting out
			// 	else{
			// 	  	printf("else SBB server received msg: [%s] from client\n", msg);
		 //        	strcpy(rmsg," this is the server message response!");
			// 		if (send(sd_current, rmsg, strlen(rmsg), 0) == -1) {
			// 			fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
			// 			exit(1);
			// 		}
			// 	}*/	
			// }
        	

		// /* 
		//  * ack back to the client 
		//  */
		// strcpy(msg," this is the server message response!");
		// if (send(sd_current, msg, strlen(msg), 0) == -1) {
		// 	fprintf(stderr,"SBB send(...) failed errno: %d exiting...\n", errno);
		// 	exit(1);
		// }
        memset(&msg[0], 0, sizeof(msg));
        memset(&rmsg[0], 0, sizeof(rmsg));
        //memset(&elapsedTime[0], 0, sizeof(elapsedTime));
	}

	if( 0 == ret ) {
		printf("SBB ciient exited...\n");
		/* For TCP sockets	
		 * the return value 0 means the peer has closed its half side of the connection 
		 */
	}
	else if( -1 == ret ) {
		fprintf(stderr,"SBB recv(...) returned failed - errno: %d exiting...\n", errno);	
		exit(1);
	}

	close(sd_current); 
	//END_TIMER(aClock);
}//close of while(1)
	close(sd);
	//END_TIMER(aClock);
}
