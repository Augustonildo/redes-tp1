#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_RACK_NUMBER 4
#define MAX_RACK_SIZE 3
#define SWITCH_TYPES 4

struct rack
{
  int switchCount;
  int installedSwitches[SWITCH_TYPES];
};

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

int endCommandHandler(char *splittedCommand)
{
  splittedCommand = strtok(NULL, " ,.-");
  if (splittedCommand != NULL)
  {
    return closeHandler(splittedCommand);
  }
  return 0;
}

int addHandler(char *splittedCommand)
{
  printf("An add!\n");
  splittedCommand = strtok(NULL, " ,.-");
  if (strcmp(splittedCommand, "sw") == 0)
  {
    printf("An sw!\n");
    splittedCommand = strtok(NULL, " ,.-");
    int switchId = atoi(splittedCommand);
    if (switchId < 1 || switchId > SWITCH_TYPES)
    {
      closeHandler(splittedCommand);
    }
    printf("Switch %d installed", switchId);
  }
  else
  {
    closeHandler(splittedCommand);
  }
  return 0;
}

int lsHandler(char *splittedCommand, struct rack *racks)
{
  splittedCommand = strtok(NULL, " ,.-");
  int rackId = atoi(splittedCommand);
  printf("%d\n", rackId);
  if (rackId < 1 || rackId > MAX_RACK_NUMBER)
  {
    printf("error rack doesn't exist\n");
    return 0;
  }

  if (racks[rackId - 1].switchCount == 0)
  {
    printf("empty rack\n");
    return 0;
  }

  for (int i = 0; i < MAX_RACK_SIZE; i++)
  {
    int found = 0;
    if (racks[rackId - 1].installedSwitches[i])
    {
      found++;
      printf("0%d", i + 1);
      if (found != racks[rackId - 1].switchCount)
      {
        printf(" ");
      }
      else
      {
        printf("\n");
      }
    }
  }
  return endCommandHandler(splittedCommand);
}

int handleCommands(char *buf, struct rack *racks)
{
  char *splittedCommand = strtok(buf, " ");
  printf("[TESTE] Next command: %s\n", splittedCommand);

  if (strcmp(splittedCommand, "add") == 0)
  {
    return addHandler(splittedCommand);
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
    return lsHandler(splittedCommand, racks);
  }
  else
  {
    return closeHandler(splittedCommand);
  }
  return 0;
}

void initializeRacks(struct rack *racks)
{
  for (int i = 0; i < MAX_RACK_NUMBER; i++)
  {
    racks[i].switchCount = 0;
    for (int j = 0; j < MAX_RACK_SIZE; j++)
    {
      racks[i].installedSwitches[j] = 0;
    }
  }
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

  struct rack racks[MAX_RACK_NUMBER];
  initializeRacks(racks);
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

      int resultHandler = handleCommands(buf, racks);
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