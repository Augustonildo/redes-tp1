#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int closeHandler(char *command)
{
  if (strcmp(command, "exit") == 0)
  {
    printf("Ok! An exit!\n");
  }
  else
  {
    printf("[What?] %s\n", command);
  }

  return -1;
}

int handleCommands(char *buf)
{
  char *splittedCommand = strtok(buf, " ");
  while (splittedCommand != NULL)
  {
    printf("[TESTE] Next command: %s\n", splittedCommand);

    if (strcmp(splittedCommand, "add") == 0)
    {
      printf("An add!\n");
    }
    else if (strcmp(splittedCommand, "rm") == 0)
    {
      printf("A rm!\n");
    }
    else if (strcmp(splittedCommand, "get") == 0)
    {
      printf("A get!\n");
    }
    else if (strcmp(splittedCommand, "ls") == 0)
    {
      printf("A ls!\n");
    }
    else
    {
      return closeHandler(splittedCommand);
    }

    splittedCommand = strtok(NULL, " ,.-");
  }
  return 0;
}

int main(int argc, char *argv[])
{
  struct sockaddr_storage storage;
  server_sockaddr_init(argv[1], argv[2], &storage);

  int s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1)
  {
    logexit("socket");
  }

  struct sockaddr *addr = (struct sockaddr *)(&storage);
  if (0 != bind(s, addr, sizeof(storage)))
  {
    logexit("bind");
  }

  if (0 != listen(s, 10))
  {
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);

  while (1)
  {
    struct sockaddr_storage cstorage;
    struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    int csock = accept(s, caddr, &caddrlen);
    if (csock == -1)
    {
      logexit("accept");
    }

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);

    char buf[BUFSZ];
    while (1)
    {
      memset(buf, 0, BUFSZ);
      size_t count = recv(csock, buf, BUFSZ - 1, 0);
      buf[strcspn(buf, "\n")] = 0;
      printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

      int resultHandler = handleCommands(buf);
      printf("[Result handler] %d\n", resultHandler);
      count = send(csock, buf, strlen(buf) + 1, 0);
      if (count != strlen(buf) + 1)
      {
        logexit("send");
      }

      if (resultHandler != 0)
      {
        close(csock);
      }
    }
  }
  exit(EXIT_SUCCESS);
}