#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define READ 0
#define WRITE 1



void launchLS(int fd[2])
{
   /* ls */
   close(fd[READ]);
   dup2(fd[WRITE],STDOUT_FILENO);
   execl("/bin/ls","ls",NULL);
}

void launchSort(pid_t pid, int fd[2])
{
   /* sort */
   int fileD;
   int status;
   close(fd[WRITE]);
   if((pid = waitpid(pid, &status, 0)) < 0)
   {
      perror(NULL);
   }
   fileD = open("outfile",O_WRONLY | O_CREAT | O_TRUNC, 0666);
   if(fileD == -1)
   {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   dup2(fileD,STDOUT_FILENO);
   dup2(fd[READ],STDIN_FILENO); 
   execl("/bin/sort","sort","-r",NULL);
}
int main (int argc, char *argv[])
{
   pid_t pid;
   int fd[2];

   /* check & make pipe */
   if(pipe(fd) == -1)
   {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   /* main fork */ 
   pid = fork();
   if(pid == 0)
   {
      launchLS(fd);
   }
   else if(pid < 0)
   {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   else 
   {
      launchSort(pid, fd);
   }
   return EXIT_SUCCESS;
}
