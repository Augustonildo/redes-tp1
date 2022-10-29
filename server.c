#include "common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_RACK_NUMBER 4
#define RACK_SIZE_LIMIT 3
#define MAX_SWITCH_TYPES 4
#define MULTIPLE_SWITCH_GET 2

struct rack
{
  int switchCount;
  int installedSwitches[MAX_SWITCH_TYPES];
};

int isValidRack(int rackId)
{
  if (rackId < 1 || rackId > MAX_RACK_NUMBER)
  {
    printf("error rack doesn't exist\n");
    return -1;
  }
  return 1;
}

int isValidSwitch(int switchId)
{
  if (switchId < 1 || switchId > MAX_SWITCH_TYPES)
  {
    printf("error switch doesn't exist\n");
    return -1;
  }
  return 1;
}

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
  splittedCommand = strtok(NULL, " ,.-");
  if (strcmp(splittedCommand, "sw") == 0)
  {
    splittedCommand = strtok(NULL, " ,.-");
    int switchId = atoi(splittedCommand);
    if (!isValidSwitch(switchId))
      closeHandler(splittedCommand);

    printf("Switch %d installed", switchId);
  }
  else
  {
    closeHandler(splittedCommand);
  }
  return 0;
}

int getHandler(char *splittedCommand, struct rack *racks)
{
  int getList[MULTIPLE_SWITCH_GET];
  int countGet = 0;

  splittedCommand = strtok(NULL, " ,.-");
  while (strcmp(splittedCommand, "in") != 0 && countGet < MULTIPLE_SWITCH_GET)
  {
    getList[countGet] = atoi(splittedCommand);
    if (!isValidSwitch(getList[countGet]))
      return 0;
    countGet++;

    splittedCommand = strtok(NULL, " ,.-");
  }

  if (countGet == 0)
    return closeHandler(splittedCommand);

  splittedCommand = strtok(NULL, " ,.-");
  int rackId = atoi(splittedCommand);
  if (!isValidRack(rackId))
    return 0;

  for (int i = 0; i < countGet; i++)
  {
    if (!racks[rackId].installedSwitches[getList[i]])
    {
      printf("error switch doesn't exist");
      return 0;
    }
    else
    {
      printf("%d Kbs ", rand() % (5000)); // Arbitrary limit for rand
    }
  }
  printf("\n");
  return 0;
}

int lsHandler(char *splittedCommand, struct rack *racks)
{
  splittedCommand = strtok(NULL, " ,.-");
  int rackId = atoi(splittedCommand);
  printf("%d\n", rackId);
  if (!isValidRack(rackId))
    return 0;

  if (racks[rackId - 1].switchCount == 0)
  {
    printf("empty rack\n");
    return 0;
  }

  for (int i = 0; i < MAX_SWITCH_TYPES; i++)
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
    return getHandler(splittedCommand, racks);
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
    for (int j = 0; j < MAX_SWITCH_TYPES; j++)
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