#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <clang-c/Index.h>

#define MAX_VAR_LENGTH 128

enum MATCHING_CONDITION
{
	MATCH_DEFINE,
	MATCH_DECLARE
};

class MatchPattern
{
public:
	int cond;
	char name [MAX_VAR_LENGTH];
	CXCursor result_cursor;
	MatchPattern() { result_cursor = clang_getNullCursor(); };
};

enum CXChildVisitResult visitor_f(
        CXCursor cursor,
        CXCursor parent,
        CXClientData clientData)
{
        CXFile file;
        unsigned int line, column, offset;
	MatchPattern* mp = (MatchPattern *)clientData;

	// if invalid cursor, visit sibling
	if (clang_Cursor_isNull(cursor)) {
		printf("visitor(): invalid cursor, visit sibling\n");
		return CXChildVisit_Continue;
	}

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
	printf("%s\n", info);
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
	CXTranslationUnit tu = clang_createTranslationUnit(cidx, ast_files[0]);
	CXCursor tu_cursor = clang_getTranslationUnitCursor(tu);
	MatchPattern match_pattern;
	clang_visitChildren(tu_cursor, visitor_f, &match_pattern);
}
