/***************************************************************************
* File: hotpotato.c
* Author: Sujeet Ojha
* Procedures:
* main - function that plays "hot potato" between five child processes
		by passing messages using the message queue. The last child process to
		receive a thousand messages is announced the winner while the rest are losers.
***************************************************************************/

#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <signal.h>


/***************************************************************************
* int main( int argc, char *argv[] )
* Author: Sujeet Ojha
* Date: 24 February 2020
* Description: This function utilizes message queues to play hot potato
* between processes. Pipe is also utilized for additional communication
* between the parent and child processes. Additionally, a file
* (terminationOrder.txt) is utilized to store information regarding the
* order of loss for all child processes.
* Parameters:
* argc I/P int The number of arguments on the command line
* argv I/P char *[] The arguments on the command line
* main O/P int Status code (not currently used)
**************************************************************************/

int main(){
	
	int anonPipe1[2]; /* Variable used to store two ends of the pipe used for communication */
    FILE *fileStore = fopen("terminationOrder.txt", "w"); /* Opening file in read mode */
    int parentID = getpid(); /* Storing the parent PID */
    
    printf("\n-------------------------------------\n");
    
    if (pipe(anonPipe1) == -1){ /* Piping and checking error conditions */
    
    	printf("Pipe has failed for anonPipe1\n");
    	
    }
    else{
    
    	printf("Pipe passed for anonPipe1\n");
    }
	
	printf("-------------------------------------\n\n\n");
	
    pid_t a; /* Creating a pid_t variable used to store fork() information */
    
    
	char* msgptr = "testmsg"; /* msgptr represents the message that will be passed between processes */
	mqd_t msgQueue; /* Message queue variable */
	struct mq_attr attributes = {.mq_maxmsg = 10, .mq_msgsize = 50}; /* Message queue attributes used for opening the queue */
	char buffer[attributes.mq_msgsize]; /* Creating a char buffer of size 50 */
	int priority = rand () % 7;	/* Defines the priority that will be assigned to messages */

	strncpy(buffer, msgptr, sizeof(buffer)); /* Copying the message onto the buffer */
	
	printf("-------------------------------------\n");

    if ((msgQueue = mq_open("/sxo160430.txt", O_CREAT | O_RDWR, 0666, &attributes)) == -1){ /* Opening the message queue in read write mode */
    
        printf("FAILURE: Message queue cannot be opened\n");        
    }
    else{
    
    	printf("SUCCESS: Message queue opened successfully\n");
    
    }

	printf("-------------------------------------\n\n\n");

    for (int i = 0; i < 5; i++){ /* This function is used to create five child processes */
    	
    	a = fork(); /*Creates a new process and returns the process ID (PID) of the child process */

   		if (a == 0){ /* If the PID relates to one of a child process */
		
			printf("-------------------------------------\n");
			printf("**** Child PID: %d ****\n\n", getpid());
		    break; /* Used to stop the process from duplicating	*/
		    		
		}
		
		else if (a > 0){ /* If parent process, do nothing */
				
			
		}
		
		else{ /* If a < 0, fork() could not create a new process, therefore, output an error */
			
			printf("Process creation has failed!\n");			
		}

	}
	
	
	if (getpid() == parentID){ /* If 5 child processes have been created and the parent process is executing */
		
		int pipeMsgReceived = 0; /* Counter variable used to determine how many processes are out of the game */
		
		printf("\n-------------------------------------");
		
		printf("\nAll 5 child processes have been created\n");
		
		if (mq_send(msgQueue, buffer, sizeof(buffer), priority) == -1){ /* The parent passing the first message to start the game */
			
			printf("FAILURE: Message send failed by parent\n");
			exit(0);
		
		}
		else{
		
			printf("SUCCESS: Message send passed by parent\n");
			
		}
		
		char pipeTestRead[100]; /* Test variable for reading pipe messages */
		close(anonPipe1[1]); /* Closing writing end of the pipe */
		
		while (pipeMsgReceived < 5) /* Loop executes until all children have sent pipe message */
		{
			if(read(anonPipe1[0], pipeTestRead, 100) != -1){ /* If a pipe message is received */
			
				pipeMsgReceived++; /* Increment counter */
			
				printf("SUCCESS: PIPE message received was: %s\n", pipeTestRead);
			
				if (pipeMsgReceived != 5){ /* If the pipe message received isn't from the winner, send a message onto the message queue */
					
					if (mq_send(msgQueue, buffer, sizeof(buffer), priority) == -1){ /* Sending a new message queue message */
			
						printf("PIPE REPLY FAILURE: MSQ Message send failed after PIPE Msg received\n");
		
					}
					
					else{
				
						printf("PIPE REPLY SUCCESS: MSQ Message sent after PIPE Msg received\n");
			
					}
				
				
				}
				else{ /* If winner is found, no need to send a new message */
					
					printf("This was the final process! No more processes left. Winner!\n"); /* Declare the final process a winner */
				
				}
			
			}
	
		}
		
		for (int i = 0; i < 5; i++){ /* Wait for all processes to finish */
			wait(NULL);
		}
		
		printf("-------------------------------------\n\n");
		
	
	}
	
	else{ /* If child process */
		
		int potatoLimit = 1000; /* The count that determines if the process is out of the game */
		int numberOfMessages = 0; /* Counter variable used to store the number of messages received */
		int processPID = getpid(); /* Variable used to store PID information */
		
		while (numberOfMessages < potatoLimit){ /* Loop exits when the process receives 1000 messages */
		
			int prio;
		
			if (mq_receive(msgQueue, buffer, sizeof(buffer), &prio) == -1){ /* If message cannot be received from the queue */
			
           		printf("FAILURE: MSGQ Message can't be received by PID %d\n", getpid());
            	
       	 	}
       	 	else{ /* If received received from the queue, increment counter */
       	 	 
       	 	 	numberOfMessages++;
       	 	 	printf("SUCCESS: MSGQ Message received by PID %d\n - Count: %d\n", getpid(), numberOfMessages);
       	 	 	
       	 	
       	 	}
       	 
       	 
       	 	if (numberOfMessages < potatoLimit){ /* Send only when child process hasn't received a thousand messages */
       	 
       	 		if (mq_send(msgQueue, buffer, sizeof(buffer), priority) == 0){ /* Once a message is received from the queue, send another message to the queue */
			
					printf("SUCCESS: MSGQ Message sent by PID %d\n", getpid());
		
				}
				
				else{
		
					printf("FAILURE: MSGQ Message can't be sent by PID %d\n", getpid());
			
				}
       	 
       	 
       		}	
		
			else if (numberOfMessages == potatoLimit){ /* If a thousand messages is received, send a pipe message to the parent */
			
				printf("\nPID: %d is out! -- Sending PIPE information\n\n", getpid());
				char pipeTest[] = "pipeTestingMessage"; /* Test message sent to the pipe */
				close(anonPipe1[0]); /* Closing reading end of the pipe */
				write(anonPipe1[1], pipeTest, sizeof(pipeTest)); /* Writing to the pipe */
				fprintf(fileStore, "PID: %d is out!\n", getpid()); /* Storing PID information to the file once the process is out of the game */
		
			}
		
			
		}
			
	}
	
	fclose(fileStore); /* Closing the file */
	mq_close(msgQueue); /* Closing the message queue */
    mq_unlink("/sxo160430.txt"); /* Unlinking the message queue */


}