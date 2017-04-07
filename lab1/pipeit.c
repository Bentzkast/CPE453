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

void launchSort(int fd[2])
{
   /* sort */
   int fileD;
   close(fd[WRITE]);
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
   int status;

   /* check & make pipe */
   if(pipe(fd) == -1)
   {
      perror(NULL);
      exit(EXIT_FAILURE);
   }
   /* first fork for ls*/ 
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
      /* second child for sort */
      pid = fork();
      if(pid == 0)
      {
         launchSort(fd);
      }
      else if(pid < 0)
      {
         perror(NULL);
         exit(EXIT_FAILURE);
      }
      else
      {
         close(fd[WRITE]);
         close(fd[READ]);
         if((pid = waitpid(pid, &status, 0)) < 0)
         {
            perror(NULL);
         }
      }
   }
   return EXIT_SUCCESS;
}
