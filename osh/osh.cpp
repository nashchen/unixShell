#include "comm.h"

int main(int argc,char ** argv){ 
	int count=0,status=0;
	char **pipeMsg;
	int pipeStopFlag=0;
	bool exitloop = false;
	
	//four ptr ,head ,current,next,the end of the command link
	Command *head = NULL,*thisptr=NULL,*Next=NULL,*pipeEndCmd=NULL;

	//start the cycle until exitloop is true
	while(false == exitloop){
		//here we get a commond!
		if(1 == GetCommandChain(&head)){
		    printf("get CommandChain error!\n");
		    exitloop=true;
		}
		thisptr = head;
		/*Two-way linked list traversal*/
	    while(thisptr != NULL){
			//check the thisptr->file is NULL ,if it is,quit this commond cycle,output the error msg
			if(thisptr->file == NULL ){
				
				break;
			}
			
    		//if the thisptr commond is "exit",then  exit the program!
	        if( memcmp(thisptr->file,"exit",4) == 0 && strlen(thisptr->file) == 4){
	           	exitloop = true;
	        }
	        

			//check that if the thisptr->symbolType is Pipe,fist deal the pipe
			int pipeCount=0;
			pipeEndCmd = thisptr;

			while(pipeEndCmd->symbolType == Pipe){
				pipeEndCmd = pipeEndCmd->next;
				pipeCount++;
			}

			if(pipeCount>0){
				if(pipeEndCmd -> symbolType == RedirectIn){
					printf("Ambiguous input redirect.\n");
					break;
				}
			}

			pipeCount++;
			//printf("pipeEndCmd pipeCount=%d\n",pipeCount);
			//printf("thisptr->symbolType=[%d]\n",thisptr->symbolType);
			//printf("thisptr->file=[%s]\n",thisptr->file);
			/*
			if(NULL != thisptr->arglist){
				for(count = 0; NULL != thisptr->arglist[count]; count++){
					printf("thisptr->arglist[%d]%s \n", count,thisptr->arglist[count]);
				}
			}
			*/
			//check and execute  pipe command
			if(pipeCount>1 && thisptr->symbolType == Pipe){
				int pipefd[MaxPipeNumber][2]; 
				pid_t pipepid[MaxPipeNumber];
				int i=0,j=0;
				/* fork count child process and count pipes */  
				for(i = 0; i < pipeCount; i++){
					if(pipe(pipefd[i]) < 0){
						perror("pipe");  
						break;
					}
					
			        if((pipepid[i] = fork()) < 0){
			            perror("fork()");
			            return -1;
			        }
			        else if(pipepid[i] == 0){  
			            /* child i ,three situations and different way:the first child,the last child,*/	
			            if(i == 0){
			                close(pipefd[i][0]); /* close read and redirect output*/  
			                if(dup2(pipefd[i][1], STDOUT_FILENO) < 0){  
			                    perror("dup2 failed");  
			                    return -1;  
			                }  
			            }
			            else if(i == pipeCount - 1){ 
			                for(j = 0; j < i - 1; j++){
			                	//close the unused pipefd
			                    close(pipefd[j][1]);
			                    close(pipefd[j][0]);
			                }
			                //close prev write
			                close(pipefd[j][1]); 
			                //close curr read
			                close(pipefd[i][0]);
			  				//dup2 stdin
			                if(dup2(pipefd[j][0], STDIN_FILENO) < 0){  
			                    perror("dup2 failed");  
			                	return -1;
			                }
			            }
			            else{ /* close unused pipefd */  
			                for(j = 0; j < i - 1; j++){
			                    close(pipefd[j][1]);  
			                    close(pipefd[j][0]);  
			                }  
			                //close prev write
			                close(pipefd[j][1]); 
			                //close curr read 
			                close(pipefd[i][0]);  
			  				//dup2 stdin
			                if(dup2(pipefd[j][0], STDIN_FILENO) < 0){
			                    perror("dup2 failed");  
			                    return -1;  
			                }  
			                //dup2 stdout
			                if(dup2(pipefd[i][1], STDOUT_FILENO) < 0){  
			                    perror("dup2 failed");  
			                    return -1;  
			                }  
			            }  
			            //pipe output
			            if(thisptr->symbolType == RedirectOut){
							if(thisptr -> next ->file == NULL ){
									printf("file is NULL\n");
									break;
							}
							int fd = open(thisptr -> next ->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
							if(fd == -1)
								perror("creat");

							dup2(fd,STDOUT_FILENO);
							if(execvp(thisptr->file, thisptr->arglist) < 0){
									perror("execvp");
									exit(1);
							}
						    close(fd);
			            }
			            else{
							if(execvp(thisptr->file, thisptr->arglist) < 0){
								perror("execvp");
								exit(1);
							}
			            }
			            /*child process exit */
		            	exit(0);
			        }
					else{
        				thisptr=thisptr->next;
					}
				}
				
			    for(i = 0; i < pipeCount; i++){  
			        // close all pipe fd
			        close(pipefd[i][0]);  
			        close(pipefd[i][1]);      
	    		}
    		
	    		for(i = 0; i < pipeCount; i++){
	    			//here ,wait for child process exit!
			        wait(&status);
			        if(status != 0){  
			        	//break;
		        	}
				}
				//exit the pipe cycle!
				if(pipeEndCmd->symbolType == Null || pipeEndCmd->symbolType == RedirectOut)
					break;
			}
			
			if(pipeCount>1){
				if(status !=0 && thisptr -> prev->symbolType == ExecuteOnSuccess)
					break;
				if(status == 0 && thisptr -> prev->symbolType == ExecuteOnFailure)
					break;
				pipeCount=0;//reset the numbers of the pipes
			}

			//printf("finished the pipe command!\n");
			pid_t fpid = fork();
	     	if (fpid == 0){
				//check this command->symbolType,then execute current or the next command
				switch(thisptr->symbolType){
					//child : current command file is RedirectIn
					case RedirectIn:{
						Next = thisptr -> next;
						/*recombinationCommond*/
						int ret = recombinationCommond(Next->arglist,thisptr->arglist);
						if(Next->symbolType == RedirectOut){
							Command *nNext=NULL;
							nNext = Next -> next;
							if(nNext->file == NULL ){
									printf("Ambiguous output redirect\n");
									break;
							}
							int fd = open(nNext->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
							if(fd == -1)
								perror("open");
							//redirect STDOUT_FILENO to fd;
							if(dup2(fd,STDOUT_FILENO) == -1)
		         				perror("dup2 error!");
		         			close(fd);
		         			nNext = NULL;//reset
						}
							
		         		//printf("going here pipeEndCmd\n");
						ret = execvp(thisptr->file, thisptr->arglist);
						if(ret == -1)
							printf("commond not found");
						break;
					}
					//child : current command file is RedirectOut
					case RedirectOut: { 
						Next = thisptr -> next;
						if(Next->file == NULL ){
							printf("no such command!\n");
							break;
						}
						int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
						if(fd == -1)
							perror("creat");
						dup2(fd,STDOUT_FILENO);
						int ret = execvp(thisptr->file, thisptr->arglist);
						if(ret == -1)
		         			printf("commond not found");
			         	close(fd);
			         	Next = NULL;//reset
						break;
					}
					//child : current command file is RedirectOutAppend
					case RedirectOutAppend: {
						Next = thisptr -> next;
						int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
						if(fd == -1)
							perror("open file");
						if(Next->file == NULL ){
								printf("command is NULL\n");
								break;
						}
						dup2(fd,STDOUT_FILENO);
						int ret = execvp(thisptr->file, thisptr->arglist);
						if(ret == -1)
		         			printf("commond not found");
			         	close(fd);
			         	Next = NULL;//reset
						break;
					}
					//child : current command file is ExecuteOnSuccess
					case ExecuteOnSuccess:{
		         		execvp(thisptr->file, thisptr->arglist);
						break;
					}
					//child : current command file is ExecuteOnFailure
					case ExecuteOnFailure:{
		         		execvp(thisptr->file, thisptr->arglist);
						break;
					}
					//child : current command file is Semicolon
					case Semicolon:{
		         		execvp(thisptr->file, thisptr->arglist);
						break;
					}
					
					//child : current command file is Null
					case Null:{
						int ret = execvp(thisptr->file, thisptr->arglist);
			         	if(ret == -1){
					        if( strncmp(thisptr->file,"exit",4) == 0 && strlen(thisptr->file) == 4){
					        	 break;
					        }
			         		printf("commond not found");
			         	}
						break;
					}
					//the other command.
					default:
						;
				}
	         	exit(1);//Rule 3: Always exit in child
	     	}//#end child process
	     	else if (fpid > 0){
	     		//wait child process exit
	     		wait(&status);
	     		//Judge the child process execution results
	         	switch(thisptr->symbolType){
					//parent : curent command is RedirectIn
					case RedirectIn:{
						thisptr = thisptr->next;
						if(thisptr->symbolType = RedirectOut){
							pipeStopFlag = 1;
						}
						break;
					}
					//parent : curent command is RedirectOut
					case RedirectOut:{
						Next = thisptr->next;
						if(Next->symbolType == Pipe){
							printf("Ambiguous output redirect.\n");	
							pipeStopFlag = 1;
						}
						thisptr = thisptr->next;
						break;
					}
					//parent : curent command is RedirectOutAppend
					case RedirectOutAppend:	{
						thisptr = thisptr->next;
						break;
					}
					//parent : curent command is Null,end
					case Null:{
						break;
					}
				}
	     	}
	     	else{
        		perror("fork()");
        		exit(1);
	     	}
			if(pipeStopFlag == 1){
				pipeStopFlag=0;
				break;
			}
			//if current operator is "&&" ,and current command ExecuteOnSuccess,exit
			if(	thisptr->symbolType	== ExecuteOnSuccess && (status != 0)){
				break;
			}
			//if current operator is "||" ,and current command ExecuteOnFailure,exit
			if(	thisptr->symbolType	== ExecuteOnFailure && (status == 0)){
				break;
			}
			thisptr = thisptr->next;
        // -- create process, setup inprocess communication (file handles for input/output) etc
        // -- and other process management tasks
    }
    //finished!
    DeleteCommandChain(head);
  }

  return 0;
}


