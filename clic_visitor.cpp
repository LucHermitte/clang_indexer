#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main (int argc, char* argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage:\n\t <-s ast_file>[,ast_file]...\n");
		return 1;
	}

	int c;
	char* ast_files[128];
	char* p;
	char* tmp_files;


	while ((c = getopt(argc, argv, "s:")) != -1) {
		switch (c) {
		case 's':
			tmp_files = optarg;
			break;
		default:
			printf("unknow option -%c.\n", optopt);
		}
	}

	p = strtok(tmp_files, ",");
	int fcnt = 0;
	while (p != NULL) {
		ast_files[fcnt++] = p;
		p = strtok(NULL, ",");
	}

        // debug pring
	printf("total %d ast files:\n", fcnt);
	for (int i=0; i<fcnt; i++)
	        printf("\t%s\n", ast_files[i]);
}
