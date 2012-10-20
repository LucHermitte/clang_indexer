#include "clic_printer.hpp"
#include "types.hpp"

extern "C" {
#include <clang-c/Index.h>
}
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <sstream>

enum QueryType {
	QUERY_DECL,
	QUERY_DEFINE
};

int main(int argc, char* argv[]) {
	if (argc < 6) {
		fprintf(stderr, "Usage:\n\t <-f file> <-l line> <-c col> <-i> <-p path_lists> <-x extra_files>\n");
		return 1;
	}

	char* file_name;
	int col;
	int line;
	int query_type = QUERY_DECL;
	char*p = NULL;
	char* tmp_paths = NULL;
	char* tmp_files = NULL;
	char* paths_list[128];
	char* extra_files[128];

	// get options
	int c;
	while ((c = getopt(argc, argv, "f:l:c:ip:x:")) != -1) {
		switch (c) {
		case 'f':
			file_name = optarg;
			break;
		case 'l':
			line = atoi(optarg);
			break;
		case 'c':
			col = atoi(optarg);
			break;
		case 'i':
			query_type = QUERY_DEFINE;
			break;
		case 'p':
			tmp_paths = optarg;
			break;
		case 'x':
			tmp_files = optarg;
			break;
		default:
			printf("unknow option -%c.\n", optopt);
		}
	}
			    
	// split path lists
	p = strtok(tmp_paths, ":");
	int pcnt = 0;
	while (p != NULL) {
		paths_list[pcnt++] = p;
		p = strtok(NULL, ":");
	}
	    
	p = strtok(tmp_files, ",");
	int fcnt = 0;
	while (p != NULL) {
		extra_files[fcnt++] = p;
		p = strtok(NULL, ",");
	}

	// debug print    
	std::cout << "file = " << file_name << "\n";
	std::cout << "line = " << line << ", col=" << col << "\n";
	std::cout << "type = " << query_type << "\n";
	std::cout << "total " << pcnt << " path lists \n";
	for (int i=0; i<pcnt; i++)
		std::cout << paths_list[i] << "\n";
	std::cout << "total " << fcnt << " extra files \n";
	for (int i=0; i<fcnt; i++)
		std::cout << extra_files[i] << "\n";
	

	// Set up the clang translation unit
	CXIndex cxindex = clang_createIndex(0, 0);
	const char* command_line_args[3] = {
		"-I/usr/lib/gcc/i486-linux-gnu/4.7/include", 
		"-I/usr/GNUstep/Local/Library",
		"-I/usr/GNUstep/System/Library/Makefiles/TestFramework"
	};

	// add extra files for parsing
	for (int i = 0; i<fcnt; i++) {
		struct stat sts;
		if (stat(extra_files[i], &sts) == 0) {
			printf ("Parsing %s ...\n", extra_files[i]);
			clang_createTranslationUnitFromSourceFile(cxindex, extra_files[i],
								  3, command_line_args,
								  0, 0);
		}
	}

	CXTranslationUnit tu = clang_parseTranslationUnit(
		cxindex, file_name,
		command_line_args, 3,
		0, 0,
		CXTranslationUnit_DetailedPreprocessingRecord);


	// Print any errors or warnings
	int n = clang_getNumDiagnostics(tu);
	if (n > 0) {
		int nErrors = 0;
		for (unsigned i = 0; i != n; ++i) {
			CXDiagnostic diag = clang_getDiagnostic(tu, i);
			CXString string = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
			fprintf(stderr, "%s\n", clang_getCString(string));
			if (clang_getDiagnosticSeverity(diag) == CXDiagnostic_Error
			    || clang_getDiagnosticSeverity(diag) == CXDiagnostic_Fatal)
				nErrors++;
		}
	}

	CXFile cx_file = clang_getFile(tu, (const char*)file_name);
	CXSourceLocation cx_source_loc = clang_getLocation(tu, cx_file, line, col);
	CXCursor cx_cursor = clang_getCursor(tu, cx_source_loc);

	CXCursor t_cursor = clang_getNullCursor();
	if (query_type == QUERY_DECL)
		t_cursor = clang_getCursorReferenced(cx_cursor);
	else // QUERY_DEFINE
		t_cursor = clang_getCursorDefinition(cx_cursor);
	
	CXFile t_file;
	unsigned int t_line, t_col, t_offset;

	clang_getExpansionLocation(
		clang_getCursorLocation(t_cursor),
		&t_file, &t_line, &t_col, &t_offset);

	if (clang_getFileName(t_file).data) {
		printf("target file: %s, line: %d, col: %d\n",
		       clang_getCString(clang_getFileName(t_file)),
		       t_line,
		       t_col);

		CXString t_spell = clang_getCursorKindSpelling(clang_getCursorKind(t_cursor));
		CXString t_usr = clang_getCursorUSR(t_cursor);
		printf("%s, display = %s\n",
		       clang_getCString(t_spell),
		       clang_getCString(t_usr));
		clang_disposeString(t_spell);
		clang_disposeString(t_usr);
	}
	else printf("no target file found.\n");

	// dealloc
	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(cxindex);

	return 0;
}
