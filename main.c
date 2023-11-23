#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int help() {
    printf(
" ____  _  _  _  _  __   \n"
"(  __)( \\/ )( \\/ )(  )  \n"
" ) _) / \\/ \\/ \\/ \\/ (_/\\\n"
"(____)\\_)(_/\\_)(_/\\____/\n"
"=====================\n"
"The educational Memory Management Language\n"
"====<OPTIONS>===\n"
"  -h : open this menu and exit\n"
"  -o <file name> : set the name of the output file\n"
"================\n");
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int sz, lsz, i;
    char *content, *compiled, *ofile;
    struct token **lexed;
    struct AST *parsed;
    if(argc < 2) {
        printf("Usage: %s <input file> [OPTIONS]\n");
        help();
        exit(-1);
    }
    for(i = 0; i < argc; i++) {
        if(argv[i][0] == '-') switch(argv[i][1]) {
        case 'o': ofile = argv[i+1]; break;
        case 'h': help(); exit(0); break;
        default: printf("invalid argument: %s\n", argv[i]);
        }
    }
    fp = fopen(argv[1], "r");
    content = malloc(sz = (fseek(fp, 0L, SEEK_END), ftell(fp)));
    rewind(fp);
    fread(content, 1, sz, fp);
    fclose(fp);    

    printf("Input file contents:\n%s\nlexing...\n", content);
    lexed = lex(content, &lsz);
    printf("Finished lexing!\nLex contents:\n");
    for(i=0;i < lsz;i++) reptok(*lexed[i], i);
    printf("parsing...\n");
    parsed = parse(lexed, lsz);
    printf("Finished parsing!\nParse contents:\n");
    repast(parsed, 0);
    printf("compiling...\n");
    compiled = compile(parsed);
    printf("Finished compiling!\nCompile contents:\n%s\n", compiled);
    printf("Creating %s...\n", ofile);
    fp = fopen(ofile, "w");
    fwrite(compiled, 1, strlen(compiled), fp);
    fclose(fp);
    printf("Exiting...\n");

    LEX_DEL(lexed, lsz);
    free(content);
    return 0;
}
