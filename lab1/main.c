#include <stdio.h>
#include <string.h>

#define IN 1
#define OUT 0

int fileSize(char fileName[]) {

    FILE* fileReader = fopen(fileName, "rb");

    if (fileReader == NULL) {
        printf("No Such File Found\n");
        return -1;
    }

    fseek(fileReader, 0L, SEEK_END);
    int size = ftell(fileReader);
    fclose(fileReader);

    return size;
}

int wordsCounter(char fileName[]) {

    FILE* fileReader = fopen(fileName, "r");

    if (fileReader == NULL) {
        printf("No Such File Found\n");
        return -1;
    }

    int wordsCnt = 0;
    int state = OUT;

    for (char c = getc(fileReader); c != EOF; c = getc(fileReader)) {
        if (c == ' ' || c == '\t' || c == '\n') state = OUT;
        else if (state == OUT){
            state = IN;
            ++wordsCnt;
        }
    }

    fclose(fileReader);

    return wordsCnt;
}

int linesCounter(char fileName[]) {
    FILE* fileReader = fopen(fileName, "r");

    if (fileReader == NULL) {
        printf("No Such File Found\n");
        return 0;
    }

    int empty = 1, cnt = 0, c;

    while ((c = fgetc(fileReader)) != EOF) {
        empty = 0;
        if (c == '\n') {
            cnt++;
            empty = 1;
        }
    }

    if (!empty) cnt++;

    fclose(fileReader);
    return cnt;
}

int main(int argc, char* argv[]) {

    if (argc != 3) printf("Three arguments required.");
    else {
        if (strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--bytes") == 0) printf("%d", fileSize(argv[2]));

        if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "--words") == 0) printf("%d", wordsCounter(argv[2]));

        if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--lines") == 0) printf("%d", linesCounter(argv[2]));
    }
    return 0;
}
