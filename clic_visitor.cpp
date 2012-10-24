#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <clang-c/Index.h>

#define MAX_VAR_LENGTH 128

enum MATCHING_TYPE
{
	MATCH_DEFINE,
	MATCH_DECLARE
};

struct MatchPattern
{
	int type;
	char name [MAX_VAR_LENGTH];
	CXCursor result_cursor;
};


void init_match_pattern(MatchPattern* m,
			enum MATCHING_TYPE type,
			const char* name)
{
	m->type = type;
	strncpy(m->name, name, MAX_VAR_LENGTH);
	m->result_cursor = clang_getNullCursor();
}


bool compare_match_pattern(MatchPattern* a, MatchPattern* b) {
	return ((a->type == b->type) &&
		(strcmp(a->name, b->name) == 0));
}


enum CXChildVisitResult visitor_f(
        CXCursor cursor,
        CXCursor parent,
        CXClientData clientData)
{
        CXFile file;
        unsigned int line, column, offset;
	MatchPattern* mp = (MatchPattern *)clientData;

        // if corresponding file is invalid,
	// visit sibling
	clang_getInstantiationLocation(
                clang_getCursorLocation(cursor),
                &file, &line, &column, &offset);

        if (!clang_getFileName(file).data) {
		printf("visitor(): no file data, visit sibling\n");
		return CXChildVisit_Continue;
	}

	const char* info = clang_getCString(clang_getCursorSpelling(cursor));
	if ((strlen(info) > 0) && (strcmp(mp->name, info) == 0)) {
		const char* kind = clang_getCString(clang_getCursorKindSpelling(clang_getCursorKind(cursor)));
		printf("(%d, %d) %s\t: %s\n", line, column, kind, info);
	}
        return CXChildVisit_Recurse;
}

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
	CXTranslationUnit tu_array[64];

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

        // debug print
	printf("total %d ast files:\n", fcnt);
	for (int i=0; i<fcnt; i++)
	        printf("\t%s\n", ast_files[i]);

	CXIndex cidx = clang_createIndex(0, 0);
	MatchPattern m;
	init_match_pattern(&m, MATCH_DEFINE, "dump");

	for(int i=0; i<fcnt; i++) {
		tu_array[i] = clang_createTranslationUnit(cidx, ast_files[i]);
		CXCursor current_tu_cursor = clang_getTranslationUnitCursor(tu_array[i]);
		clang_visitChildren(current_tu_cursor, visitor_f, &m);
	}
}
