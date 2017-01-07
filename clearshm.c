#include <stdlib.h>
#include <stdio.h>

main(argv, argc)
int argv;
char **argc;
{
    FILE *fp;
    int  iRet, iClient = 0;
    char sBuff[4][255];
  
    strcpy(sBuff[1], getenv("LOGNAME"));
    sprintf(sBuff[0], "ipcs -m|grep %s", sBuff[1]);
    if ((fp = popen(sBuff[0], "r")) == NULL) 
    {
        fprintf(stderr, "clearshm error!\n");
        exit(1);
    }

    while(!feof(fp))
    {
        fscanf(fp, "%s %s %s", sBuff[0], sBuff[1], sBuff[2]);
        if (sBuff[1][0]>='0' && sBuff[1][1]<='9')
        {
            sprintf(sBuff[0], "ipcrm -m %s", sBuff[1]);  
            fprintf(stderr,  "ipcrm -m %s\n", sBuff[1]);  
            system(sBuff[0]);
            iClient ++;
        }
    }
    fprintf(stderr, "%d ipc shm killed!\n", iClient); 
    pclose(fp);
}
