#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma pack(1)

void throwError(char errorText[]) {
    printf("Error occurred: %s\n", errorText);
    exit(1);
}

unsigned int reverseBytes(unsigned int val) { // Because of the processor arch need to convert in LittleEndian
    return ((val >> 24) & 0x000000FF) | ((val >> 8) & 0x0000FF00) | ((val << 8) & 0x00FF0000) | ((val << 24) & 0xFF000000);
}

unsigned char* rleEncode(const char* fileContents, int fileLength, int* newFileContentsLength) {
    int i, j = 0, k = 0;
    char count;

    char* tmp = malloc(2 * fileLength * sizeof(char));
    char* encoded;

    for (i = 0; i < fileLength; i++) {
        count = 1;

        while (i + 1 < fileLength && fileContents[i] == fileContents[i + 1] && count < 127) {
            count++;
            i++;
        }
        tmp[j++] = count;
        tmp[j++] = fileContents[i];
    }

    count = 0;
    i = 0;
    encoded = malloc(2 * j * sizeof(char));

    while (i < j) {
        while (i < j && tmp[i] == 1 && count < 127) {
            count++;
            i += 2;
        }
        if (count != 0) {
            encoded[k++] = count;
            while (count > 0) {
                encoded[k++] = tmp[i - (count * 2 - 1)];
                count--;
            }
        } else {
            encoded[k++] = tmp[i] | 128;
            encoded[k++] = tmp[i + 1];
            i+=2;
        }
    }
    free(tmp);
    *newFileContentsLength = k;
    return encoded;
}

unsigned char* rleDecode(const char* fileContents, int fileLength, int* newFileContentsLength) {
    int i = 0, ulen = 0, j, k;

    while (i < fileLength) {
        if (fileContents[i] & 128) {
            ulen += fileContents[i] & 127;
            i += 2;
        } else {
            ulen += fileContents[i];
            i += fileContents[i] + 1;
        }
    }
    printf("Decoded length: %d\n", ulen);
    *newFileContentsLength = ulen;

    unsigned char* decoded = calloc(sizeof(char), ulen);
    i = 0, k = 0;

    while (i < fileLength) {
        if (fileContents[i] & 128) {
            for (j = 0; j < (fileContents[i] & 127); j++) {
                decoded[j + k] = fileContents[i + 1];
            }
            k += (fileContents[i] & 127);
            i += 2;
        } else {
            for (j = 0; j < fileContents[i]; j++) {
                decoded[j + k] = fileContents[i + j + 1];
            }
            k += fileContents[i];
            i += fileContents[i] + 1;
        }
    }
    printf("\n");
    return decoded;
}

unsigned char* toString(char* notString, int length) {
    unsigned char* string = malloc(length * sizeof(char));
    memcpy(string, notString, length);
    string[length] = '\0';
    return string;
}

// Archive header
union arcHeader {
    unsigned char buffer[8];

    struct {
        unsigned int id;			// 4 bytes 1095910216 - 41 52 43 48 - 'ARCH' in HEX
        unsigned int numberOfFiles; // 4 bytes
    } data;
};

// File header
union fileHeader {
    unsigned char buffer[8];

    struct {
        unsigned int fileNameLength;	// 4 bytes
        unsigned int fileContentLength; // 4 bytes
    } data;
};

void list(char* arcFileName) {
    FILE* file = fopen(arcFileName, "rb"); // Open file for read in binary

    if (file == NULL) {
        perror(arcFileName); // Check for success open
        exit(0);
    }

    union arcHeader arcHeader;
    fread(arcHeader.buffer, sizeof(int), 2, file);

    if (reverseBytes(arcHeader.data.id) != 1095910216) {
        throwError("Not .arc file");
    }
    int numberOfFiles = reverseBytes(arcHeader.data.numberOfFiles), i;
    printf("Number of files: %d\n", numberOfFiles);

    union fileHeader fileHeader;
    int fileNameLength, fileContentLength;
    char* fileName;
    for (i = 0; i < numberOfFiles; i++) {
        fread(fileHeader.buffer, sizeof(int), 2, file);
        fileNameLength = reverseBytes(fileHeader.data.fileNameLength);
        fileContentLength = reverseBytes(fileHeader.data.fileContentLength);

        fileName = malloc(fileNameLength * sizeof(char));
        fread(fileName, sizeof(char), fileNameLength, file);
        printf("%s size: %d\n", toString(fileName, fileNameLength), fileContentLength);

        fseek(file, fileContentLength, SEEK_CUR);

        free(fileName);
    }
}

void extract(char* arcFileName) {
    FILE* file = fopen(arcFileName, "rb"); // Open file for read in binary

    if (file == NULL) {
        perror(arcFileName); // Check for success open
        exit(0);
    }

    union arcHeader arcHeader;
    fread(arcHeader.buffer, sizeof(int), 2, file);

    if (reverseBytes(arcHeader.data.id) != 1095910216) {
        throwError("Not .arc file");
    }
    int numberOfFiles = reverseBytes(arcHeader.data.numberOfFiles), i;
    printf("Number of files to extract: %d\n", numberOfFiles);

    FILE* toExtract;
    union fileHeader fileHeader;
    int fileNameLength, fileContentLength;
    char* fileName;


    for (i = 0; i < numberOfFiles; i++) {
        fread(fileHeader.buffer, sizeof(int), 2, file);
        fileNameLength = reverseBytes(fileHeader.data.fileNameLength);
        fileContentLength = reverseBytes(fileHeader.data.fileContentLength);

        fileName = malloc(fileNameLength * sizeof(char));
        fread(fileName, sizeof(char), fileNameLength, file);
        printf("%s size: %d\n", toString(fileName, fileNameLength), fileContentLength);

        char* toDecode = malloc(fileContentLength * sizeof(char));
        int decodedLength;
        fread(toDecode, sizeof(char), fileContentLength, file);
        unsigned char* decoded = rleDecode(toDecode, fileContentLength, &decodedLength);

        toExtract = fopen(toString(fileName, fileNameLength), "wb");

        fwrite(decoded, sizeof(char), decodedLength, toExtract);
        free(fileName);
        fclose(toExtract);
    }

    fclose(file);

}

void create(char* fileName, char** fileNames, int numberOfFiles) {
    int i;

    FILE* toArchive; // Check if all files exist
    for (i = 0; i < numberOfFiles; i++) {
        toArchive = fopen(fileNames[i], "rb");
        if (toArchive == NULL) { // Check for success open
            perror(fileNames[i]);
            exit(0);
        }
        fclose(toArchive);
    }

    FILE* file = fopen(fileName, "wb"); // Open file for write in binary

    if (file == NULL) {
        perror("Error while opening file"); // Check for success open
        exit(0);
    }

    union arcHeader arcHeader;
    arcHeader.data.id = reverseBytes(1095910216);
    arcHeader.data.numberOfFiles = reverseBytes(numberOfFiles);

    fwrite(arcHeader.buffer, sizeof(int), 2, file); // Arc header

    union fileHeader fileHeader;

    for (i = 0; i < numberOfFiles; i++) {
        toArchive = fopen(fileNames[i], "rb");
        fileHeader.data.fileNameLength = reverseBytes(strlen(fileNames[i]));

        fseek(toArchive, 0, SEEK_END);
        int lengthOfFile = ftell(toArchive);
        fseek(toArchive, 0, SEEK_SET);

        // rle encode
        unsigned char* fileContents = malloc(lengthOfFile * sizeof(char));
        fread(fileContents, sizeof(char), lengthOfFile, toArchive);
        fclose(toArchive);

        int newFileLength = 0;
        unsigned char* encodedFile = rleEncode(fileContents, lengthOfFile, &newFileLength);
        // end of encode

        fileHeader.data.fileContentLength = reverseBytes(newFileLength);
        fwrite(fileHeader.buffer, sizeof(int), 2, file); // File header

        fwrite(fileNames[i], sizeof(char), strlen(fileNames[i]), file);
        fwrite(encodedFile, sizeof(char), newFileLength, file);
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    char* filepath = argv[2];

    if (!strcmp(argv[3], "--list")) {
        list(filepath); // Show list of files in arc
    } else if (!strcmp(argv[3], "--extract")) {
        extract(filepath); // Extract archive
    } else if (!strcmp(argv[3], "--create")) {
        int numberOfFiles = argc - 4, i;
        char** fileNames = malloc(numberOfFiles * (sizeof(char *)));
        for (i = 0; i < numberOfFiles; i++) {
            fileNames[i] = argv[4 + i];
        }
        create(filepath, fileNames, numberOfFiles); // Create archive
    } else {
        throwError("Unknown command");
    }
}