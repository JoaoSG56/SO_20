#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_LINE_SIZE 1024



char* createBuf(int argc, char* argv[]){
    int tam = 0;
    int i;
    int k = 0;
    for(i = 1; i < argc ; i++) tam += strlen(argv[i]);
    char * buf = malloc(sizeof(char) * (tam + argc-2 + 2)); // tamanho dos argumentos + o numero de espaços + \0
    for(i = 1 ; i<argc; i++){
        strcpy(buf+k,argv[i]);
        k += strlen(argv[i]);
        if(i != argc-1){
            buf[k++] = ' ';
        }
    }
    buf[k] = '\0';
    return buf;
}

void cleanHandler(int signum){
    _exit(0);
}

int main(int argc, char* argv[]){
    int res;
    char buf[MAX_LINE_SIZE];
    int fd_cl_sv_write, fd_sv_cl_read;
    int pid;

    if((fd_cl_sv_write = open("fifo-cl-sv",O_WRONLY)) == -1){ // open named pipe for write (cliente -> sv)
        perror("open");
        return -1;
    }
    //else 
    //    printf("[DEBUG] opened fifo cl-sv for [writing]\n");

    if((fd_sv_cl_read = open("fifo-sv-cl",O_RDONLY)) == -1){ // open named pipe for read (sv -> cliente)
       perror("open");
        return -1;
    }
    //else
    //    printf("[DEBUG] opened fifo cl-sv for [reading]\n");

    if((pid = fork()) == 0){
        signal(SIGALRM,cleanHandler);
        if(argc > 1){
            alarm(30);
            res = read(fd_sv_cl_read,buf,MAX_LINE_SIZE);
            write(1,buf,res);
        }
        
        else{
            do{
                while((res = read(fd_sv_cl_read,buf,MAX_LINE_SIZE)) > 0){ // escrever tudo que vem do pipe sv->cl no terminal
                    write(1,buf,res);
                }
                if((fd_sv_cl_read = open("fifo-sv-cl",O_RDONLY)) == -1){ // open named pipe for read (sv -> cliente)
                    perror("open");
                    return -1;
                }
            }while(1);
        }
        close(fd_cl_sv_write);
    	close(fd_sv_cl_read);
        _exit(0);
    }
    else{ // lê do ecrã, escreve no pipe (cl -> sv)
    

        if(argc > 1){ // ./prog xxx xxx xxx (linha de comandos)
     		char *a = createBuf(argc,argv);
         	write(fd_cl_sv_write,a,strlen(a));
        	//write(fd_cl_sv_write,"\n",2);
           	 	
        }

        
        else{ // ./prog -> interface interpretada

            while((res = read(0,buf,MAX_LINE_SIZE)) > 0){
                buf[res-1] = '\0';
                write(fd_cl_sv_write,buf,res);
            }
            kill(pid,SIGKILL); // quando acaba, mata o processo que está a ler do pipe -> prog termina
        }

    }
    

    close(fd_cl_sv_write);
    close(fd_sv_cl_read);

    return 0;
}