#include "routeBase.inc"

int serverCount=0;
char *configFile="./server.conf";
client_server server[MAX_SERVER];

int ReadConfig(char *configFile) {
	FILE *fp;
	int i;
	printf("configFile:%s\n",configFile);

	if ((fp = fopen(configFile, "r")) == NULL) {
		printf("Cannot open file, press any key exit!");
		getchar();
		return -1;
	}
	for (i = 0; i < MAX_SERVER; i++) {
		fscanf(fp,"%s %d", server[i].ip, &server[i].port);
	}
	for (int i = 0; i < MAX_SERVER; i++) {
		printf("%s %d\n", server[i].ip, server[i].port);
	}
	serverCount = i;
	fclose(fp);
	return 0;
}
