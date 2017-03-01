#include <fcntl.h>
#include "parse.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

//debug Moudle

//#define COMMNDLINE  1
//#define DEBUG 1
#define DEBUG11 1

void sys_err(int fd,char * err_file,const char * msg)
{
  char msgstr[256];
  memset(msgstr,0x00,sizeof(msgstr));
  sprintf(msgstr,"%s:%s\n",err_file,msg);
  write(fd,msgstr,strlen(msgstr));
}

int makeUpExeclpCommond(int argc,char **argvin,char **argvout)
{
	int i=0,j=0;
	if( argvin == NULL || argvout == NULL)
		return -1;
	for(i=0;NULL != argvout[i];i++)
	{
		 //printf("argvout[%d]%s \n", i,argvout[i]);
	}
	//printf("i=%d\n",i);
	for(j=0;NULL != argvin[j];j++)
	{
		/*
		printf("i+j=%d\n",i+j);
		printf("argvin[%d]->%s \n", j,argvin[j]);
		printf("argvout[%d]->%s \n", i+j,argvout[i+j]);	
		*/
		// errdo!  strncpy(argvout[i+j],argvin[j],strlen(argvin[j]));
		argvout[i+j] = argvin[j];
	}

	return 0;
}


int main(int argc,char ** argv)
{ 
  int count=0,ii=0;
  char tmpstr[1024*2];
  int pipefd[5][2]; 
	pid_t pid2[5];  
	int status=0;
	int startflag=0;
	//int pipefd[4];
	char **pipeMsg;
	int pipeNumber=0;
	int pipeStopFlag=0;
	
/*
	int outfd=dup(STDOUT_FILENO);
	int infd=dup(STDIN_FILENO);
	int errfd=dup(STDERR_FILENO);
*/
  memset(tmpstr,0x00,sizeof(tmpstr));
  
  Command *head = NULL,*current=NULL,*Next=NULL,*test=NULL,*nNext=NULL;

#ifdef DEBUG
  for(count=0;count < argc;count++)
  {
  	printf("[%d][%s]\n",count,argv[count]);
  }
#endif

  /*
  if(argc == 2)
  {
	  if(strncmp(argv[1],"-t",2) == 0)
	  {
	  	printf("不输出到控制台\n");	
	  }
  }
  */

	bool exitloop = false;
  while(false == exitloop)
  {
		if(status_failure == GetCommandChain(&head)) 
		{
	    write(STDOUT_FILENO,"osh: GetCommandChain error return\n",34);
	    exitloop=true;
		}
		current = head;

		/*Two-way linked list traversal*/
    while(current != NULL)
    {
    		startflag++; 	//startflag>=1;
#ifdef COMMNDLINE
				printf("============================================\n");
#endif
    		count=0;
				//check the current->file is NULL ,if it is,quit this commond cycle,output the error msg
				if(current->file == NULL )
				{
#ifdef DEBUG
						printf("file is NULL\n");
#endif
						break;
				}
    		//Check current commond is exit!
        if( strncmp(current->file,"exit",4) == 0 && strlen(current->file) == 4)
        {
        	 //break
           exitloop = true;
        }
#ifdef DEBUG
				printf("deal:SymbolType : %d\n", current->symbolType);
				printf("deal:file : [%s]\n", current->file);
				printf("deal:inFileHandle : [%d]\n", current->inFileHandle);
				printf("deal:outFileHandle : [%d]\n", current->outFileHandle);
				printf("deal:errorFileHandle : [%d]\n", current->outFileHandle);			
				if(NULL != current->arglist) 
    		{
						for(count = 0; NULL != current->arglist[count]; count++) 
						{
							 printf("arglist[%d]%s \n", count,current->arglist[count]);
						}
				}
#endif
				
				/*get  pipe number */	
				test = current;
				//printf("test start[%d]!\n",test->symbolType);
				for(pipeNumber=0;test->symbolType == Pipe;pipeNumber++)
				{
					//count++;
					test = test->next;
				}
				
#ifdef DEBUG
				printf("deal pipe before! and after pipe commond is [%d][%d][%s]\n",pipeNumber,test->symbolType,test->file);
#endif
				if(pipeNumber>0)
				{
					
					if(test -> symbolType == RedirectIn)
					{
						printf("Ambiguous input redirect.\n");
						break;
					}
				}

				pipeNumber++;
				//printf("test pipeNumber = %d\n",pipeNumber);
				
				//printf("test pipeNumber[%d][%s]\n",current->symbolType,current->file);

				//check  pipe here
				if(pipeNumber>1&&current->symbolType == Pipe)
				{
					//printf("deal pipe symbolType = %d,create %d pipes\n",current->symbolType,pipeNumber);
					
					//test = current;
					int i=0,j=0;
					/* fork count child process */  
					for(i = 0; i < pipeNumber; i++)
					{  
						if(pipe(pipefd[i]) < 0)
						{
							perror("pipe");  
							break;
						}
		        if((pid2[i] = fork()) < 0)
		        {
		            fprintf(stderr, "%s:%d: fork() failed: %s\n", __FILE__,  
		                __LINE__, strerror(errno));  
		            return -1;  
		        }
		        else if(pid2[i] == 0)
		        {  
		            /* child i */  
		            //int dup2(int oldfd, int newfd);  
		            //printf("child test:file = [%s]\n",test->file); 	
		            if(i == 0)
		            { /* the first child */  
		                close(pipefd[i][0]); /* close curr process read */  
		                if(dup2(pipefd[i][1], STDOUT_FILENO) < 0){  
		                    perror("dup2 failed");  
		                    return -1;  
		                }  
		            }
		            else if(i == pipeNumber - 1)
		            {/* the last child */  
		                for(j = 0; j < i - 1; j++){ /* close unuse pipefd */  
		                    close(pipefd[j][1]);  
		                    close(pipefd[j][0]);  
		                }  
		                close(pipefd[j][1]); /* close prev process end of write */  
		                close(pipefd[i][0]); /* close curr process end of read */  
		  
		                if(dup2(pipefd[j][0], STDIN_FILENO) < 0){  
		                    perror("dup2 failed");  
		                return -1;  
		                }
		            }
		            else
		            {
		                for(j = 0; j < i - 1; j++){ /* close unuse pipefd */  
		                    close(pipefd[j][1]);  
		                    close(pipefd[j][0]);  
		                }  
		                close(pipefd[j][1]); /* close prev process end of write */  
		                close(pipefd[i][0]); /* close curr process end of read */  
		  
		                if(dup2(pipefd[j][0], STDIN_FILENO) < 0){  
		                    perror("dup2 failed");  
		                    return -1;  
		                }  
		                if(dup2(pipefd[i][1], STDOUT_FILENO) < 0){  
		                    perror("dup2 failed");  
		                    return -1;  
		                }  
		            }  
		            //int execvp(const char *file, char *const argv[]);
		            //pipe output
		            if(current->symbolType == RedirectOut)
		            {
										Next = current -> next;

										if(Next->file == NULL )
										{
												printf("file is NULL\n");
												break;
										}
										int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
										if(fd == -1)
											perror("creat");

					         	//Next = NULL;//reset
					         	
										dup2(fd,STDOUT_FILENO);
										if(execvp(current->file, current->arglist) < 0)
										{
												fprintf(stderr, "%s:%d: fork() failed: %s\n", __FILE__,  
												__LINE__, strerror(errno));  
												exit(1);
										}
					         	close(fd);
		            }
		            else 
		            {
										if(execvp(current->file, current->arglist) < 0)
										{
											fprintf(stderr, "%s:%d: fork() failed: %s\n", __FILE__,  
											__LINE__, strerror(errno));  
											exit(1);
										}
		            }
	            	exit(0);
	            	/*child process exit */
	        	}
						else
						{
	        			current=current->next;
						}
					}
					
	        //printf("deal pipe here : after fork!\n");
			    for(i = 0; i < pipeNumber; i++)
			    {  
			        // close all pipe file descriptor 
			        close(pipefd[i][0]);  
			        close(pipefd[i][1]);      
	    		}
	    		
	    		for(i = 0; i < pipeNumber; i++)
	    		{  
		        //pid_t waitpid(pid_t pid, int *status, int options);  
		        waitpid(pid2[i], &status, 0);  
		        if(status != 0)
		        {  
		        	//printf("one error ouucred !\n");
		        	//pipeStopFlag = 2;
		        	//break;
		          //return status;  
	        	}
					}
					/**/
					//printf("the last symbolType = [%s]\n",test->symbolType);
					//exit the pipe cycle!
					if(test->symbolType == Null || test->symbolType == RedirectOut)
						break;
					/*one error ouucred !
					if(pipeStopFlag == 2)
					{
						//here need to
						//pipeStopFlag = 2;
						printf("hello world! test->symbolType1\n");//,test->symbolType
						break;
					}*/
				}
				
				if(pipeNumber>1)
				{
					//printf("deal pipe ok! ret status = %d\n",status);
					
					if(status !=0 && current -> prev->symbolType == ExecuteOnSuccess)
						break;
					if(status == 0 && current -> prev->symbolType == ExecuteOnFailure)
						break;

					pipeNumber=0;
				}
				
				//printf("hello world! test->symbolType2\n");//,test->symbolType
				
				pid_t cpid = fork();				/*Check SymbolType*/
	     	if (cpid < 0) 
	     	{
	        		sys_err(STDOUT_FILENO,"fork","fork() error!");
	        		exit(1);
	     	}
	     	else if (cpid == 0)
	     	{
#ifdef DEBUG
	     			printf("[%d]child process %d start...\n",current->symbolType,getpid());
#endif
/*

						if(startflag!=1)
						{
								if(current->prev->symbolType == Pipe)
								{
										printf("deal:SymbolType : %d\n", current->symbolType);
										printf("deal:file : [%s]\n", current->file);
									  dup2(pipefd[0], 0);
									  dup2(pipefd[3], 1);
									  close(pipefd[0]);
									  close(pipefd[1]);
									  close(pipefd[2]);
									  close(pipefd[3]);
									 	if(-1==execvp(current->file, current->arglist))
								 		printf("execvp err exit!\n");
								}
						}

						//child deal : next commond connected by pipes
						if(current->symbolType == Pipe)      		// 6 - | 
						{
								printf("child come here,current->file:%s!\n",current->file);
								dup2(pipefd[1],STDOUT_FILENO);
								close(pipefd[0]);
								close(pipefd[1]);
								close(pipefd[2]);
								close(pipefd[3]);
								if(execvp(current->file, current->arglist) < 0)
			         	{
			         		//exit! parent will get error!
			         	}
								break;
						}
*/
						switch(current->symbolType)
						{
								case RedirectIn:   					// 0 - <
								{
										Next = current -> next;

										/*
										printf("Next:file : [%s]\n", Next->file);
										printf("Next:file : [%d]\n",Next->symbolType);
										*/
										//add Next-> arglist to current->arglist
										/*makeUpExeclpCommond！*/
										int ret = makeUpExeclpCommond(1,Next->arglist,current->arglist);
#ifdef DEBUG									
										if(NULL != Next->arglist) 
						    		{
												for(count = 0; NULL != Next->arglist[count]; count++) 
												{
													 printf("Next:arglist[%d]%s \n", count,Next->arglist[count]);
												}
										}

#endif
										
#ifdef DEBUG
										if(NULL != current->arglist) 
						    		{
												for(count = 0; NULL != current->arglist[count]; count++) 
												{
													 printf("current->arglist[%d]%s \n", count,current->arglist[count]);
												}
										}
#endif

										if(Next->symbolType == RedirectOut)
										{
											//printf("< test > test2\n");
											nNext = Next -> next;
											/*
											printf("Next:file : [%s]\n", Next->file);
											printf("Next:inFileHandle : [%d]\n", Next->inFileHandle);
											printf("Next:outFileHandle : [%d]\n", Next->outFileHandle);
											printf("Next:errorFileHandle : [%d]\n", Next->outFileHandle);
											*/
											//open Next-> name
											//int outfd=dup(STDOUT_FILENO);
											if(nNext->file == NULL )
											{
													printf("output file is not exist\n");
													break;
											}
											int fd = open(nNext->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
											if(fd == -1)
												perror("creat");
											//redirect STDOUT_FILENO to fd;
	
											if(dup2(fd,STDOUT_FILENO) == -1)
						         		sys_err(STDOUT_FILENO,"<>","dup2 error!");
						         	close(fd);
						         	
						         	//nNext = NULL;//reset
										}
										
					         	//printf("going here test\n");
										ret = execvp(current->file, current->arglist);
										if(ret == -1)
					         		sys_err(STDOUT_FILENO,current->file,"commond not found");
					         	
					         	
										/*
										int outfd=dup(STDERR_FILENO);
										int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
										if(fd == -1)
											perror("creat");
										if(Next->file == NULL )
										{
												printf("file is NULL\n");
												break;
										}
										dup2(fd,STDOUT_FILENO);
										int ret = execvp(current->file, current->arglist);
										dup2(outfd,STDOUT_FILENO);
										if(ret == -1)
					         		sys_err(STDOUT_FILENO,current->file,"commond not found");
					         	close(fd);
					         	*/
					         	Next = NULL;//reset
										break;
								}
								case RedirectOut:    				// 2 - >
								{
										Next = current -> next;
										/*
										printf("Next:file : [%s]\n", Next->file);
										printf("Next:inFileHandle : [%d]\n", Next->inFileHandle);
										printf("Next:outFileHandle : [%d]\n", Next->outFileHandle);
										printf("Next:errorFileHandle : [%d]\n", Next->outFileHandle);
										*/
										//open Next-> name
										//int outfd=dup(STDOUT_FILENO);
										if(Next->file == NULL )
										{
												printf("file is NULL\n");
												break;
										}
										int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
										if(fd == -1)
											perror("creat");

										dup2(fd,STDOUT_FILENO);
										int ret = execvp(current->file, current->arglist);
										//dup2(outfd,STDOUT_FILENO);
										if(ret == -1)
					         		sys_err(STDOUT_FILENO,current->file,"commond not found");
					         	close(fd);
					         	Next = NULL;//reset
										break;
								}
								case RedirectOutAppend:    	// 3 - >>
								{
										Next = current -> next;
										/*
										printf("Next:file : [%s]\n", Next->file);
										printf("Next:inFileHandle : [%d]\n", Next->inFileHandle);
										printf("Next:outFileHandle : [%d]\n", Next->outFileHandle);
										printf("Next:errorFileHandle : [%d]\n", Next->outFileHandle);
										*/
										//open Next-> name
										int outfd=dup(STDERR_FILENO);
										int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
										if(fd == -1)
											perror("creat");
										if(Next->file == NULL )
										{
												printf("file is NULL\n");
												break;
										}
										dup2(fd,STDOUT_FILENO);
										int ret = execvp(current->file, current->arglist);
										dup2(outfd,STDOUT_FILENO);
										if(ret == -1)
					         		sys_err(STDOUT_FILENO,current->file,"commond not found");
					         	close(fd);
					         	Next = NULL;//reset
										break;
								}
								case ExecuteOnSuccess:  		// 4 - && - exec on success
								{
										//sample
				         		if(execvp(current->file, current->arglist) < 0)
					         	{
					         		//exit! parent will get error!
					         	}

										break;
								}
								case ExecuteOnFailure:  		// 5 - || - exec on failure
								{
										//sample
				         		if(execvp(current->file, current->arglist) < 0)
					         	{
					         		//exit! parent will get error!
					         	}

										break;
								}
								case Semicolon:  						// 9 ;
								{
										//sample
				         		if(execvp(current->file, current->arglist) < 0)
					         	{
					         		//exit! parent will get error!
					         	}

										break;
								}
								case Null:									// 7 - end of string (null character encountered)
								{
										/*printf("current->prev->SymbolType=[%d]\n",current->prev->symbolType);
										if(current!=head && current->prev->symbolType < 4 )
										{
											printf("exit=[%d]\n",1);
											//break;
										}
										if(pipeStopFlag == 2)
										{
											//pipe end ;
											printf("hello world! [%s]!\n",current -> file);
											pipeStopFlag = 0;
											//open Next-> name
											//int outfd=dup(STDOUT_FILENO);
											if(current->file == NULL )
											{
													printf("file is NULL\n");
													break;
											}
											int fd = open(Next->file,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
											if(fd == -1)
												perror("open filer");
	
											dup2(fd,STDOUT_FILENO);

						         	close(fd);
						         	//Next = NULL;//reset
										}
										else
										{*/
											int ret = execvp(current->file, current->arglist);
						         	if(ret == -1)
						         	{
								        if( strncmp(current->file,"exit",4) == 0 && strlen(current->file) == 4)
								        {
								        	 break;
								           //exitloop = true;
								        }
						         		sys_err(STDOUT_FILENO,current->file,"commond not found");
						         	}
					        	//}
										break;
								}
						}
	         	//fprintf(stderr, "Exec Failed \n");
#ifdef DEBUG
	         	printf("child process %d exit...\n",getpid());
#endif
	         	exit(1);//Rule 3: Always exit in child
	     	}//#end child process
	     	else
	     	{
	     		
	     			if(wait(&status))

	     			
#ifdef DEBUG
	     			printf("parent process %d waiting...\n",getpid());
	     			printf("parent wait result  %d\n",status);
#endif
	         	switch(current->symbolType)
						{
								case RedirectIn:  			// 0 - <
								{
										current = current->next;
										
					         	//printf("going here test\n");
					         	
										if(current->symbolType = RedirectOut)		
										{
											//stop cycle
											pipeStopFlag = 1;
											//current = current->next;
										}
										
					         	//printf("going here test\n");
										break;
								}
								case RedirectOut: 			// 2 - >
								{
										Next = current->next;
										if(Next->symbolType == Pipe)
										{
											printf("Ambiguous output redirect.\n");	
											//break!
											pipeStopFlag = 1;
										}
										current = current->next;
										break;
								}
								case RedirectOutAppend:	// 3 - >>
								{
										current = current->next;
										break;
								}
								case Null:							// 7 - end of string (null character encountered)
								{
										break;
								}
						}
	         	
	         	//while(wait(&status)!=pid);//parent process waiting...
	         	//execlp(....);
	         	//fprintf(stderr, "Exec Failed \n");
#ifdef DEBUG
	         	printf("parent process %d doing...\n",getpid());
#endif
	         	//parent don't exit.
	         	//Rule 4: Wait for child unless we need to launch another exec 
	     	}
				//deal <>
				if(pipeStopFlag == 1)
				{
					pipeStopFlag=0;
					break;	
				}
				
				
				//if operator is "&&" and child process output error!
				if(	current->symbolType	== ExecuteOnSuccess && (status != 0))
				{	
#ifdef DEBUG
					printf("current->symbolType	= ExecuteOnSuccess && (status != 0)\n");
#endif
					break;
				}
				if(	current->symbolType	== ExecuteOnFailure && (status == 0))
				{	
#ifdef DEBUG
					printf("current->symbolType	= ExecuteOnSuccess && (status != 0)\n");
#endif
					break;
				}
				current = current->next;

#ifdef COMMNDLINE
				printf("============================================\n");
#endif
        // -- create process, setup inprocess communication (file handles for input/output) etc
        // -- and other process management tasks
    }
		//printf("current is %d commond\n",count);
    //done with current line. delete the command chain
    DeleteCommandChain(head);
    startflag=0;
  }

  return 0;
}


