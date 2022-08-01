#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *monthsVariables[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

long long parseDate(char *data) {
    struct tm time = {
            .tm_mday = atoi(data),
            .tm_year = atoi(data + 7) - 1900,
            .tm_hour = atoi(data + 12),
            .tm_min = atoi(data + 15),
            .tm_sec = atoi(data + 18),
            .tm_isdst = -1
    };

    data[6] = '\0';

    int n = 0;

    while (strcmp(data + 3, monthsVariables[n])) {
        n++;
    }

    time.tm_mon = n;

    return mktime(&time);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Wrong command format. Try again.\n");
        return -1;
    }

    char *data;

    long long timeRange;

    data = argv[1];

    timeRange = atoll(argv[2]);

    FILE *file = fopen(data, "r");

    if (!file) {
        printf("File Error.\n");
        return -1;
    }

    long long resCapacity = 128;

    long long availableTime = 1000;

    char **res = (char **) malloc(resCapacity * sizeof(char *));

    long long *time = malloc(availableTime * sizeof(long long ));

    char str[2049];
    long long errorsCnt = 0;
    long long strCnt = 0;

    while (fgets(str, sizeof(str), file) != NULL) {

        char buf[21];
        int i = 0, j = strlen(str) - 1;

        while (str[i++] != '[') i;

        memcpy(buf, str + i, 20);
        str[20] = '\0';
        if (strCnt == availableTime) {
            availableTime = availableTime * 3 / 2 + 1;
            time = realloc(time, availableTime * sizeof(long long));
        }

        time[strCnt++] = parseDate(buf);


        while (str[j--] != ' ');

        if (str[j - 2] == '5') {
            if (errorsCnt == resCapacity) {
                resCapacity = resCapacity * 3 / 2 + 1;
                res = realloc(res, resCapacity * sizeof(char *));
            }

            while (str[i++] != '"');
            while (str[j--] != '"');
            str[j + 1] = '\0';
            res[errorsCnt] = malloc(strlen(str + i) + 1);
            strcpy(res[errorsCnt++], str + i);
        }
    }

    long long left = 0, currentTimeRange = 0;
    long long firstMark = time[0], secondMark = time[0];
    long long start = 0, end = 0;
    long long reqCnt = 0, maxReqCnt = 0;

    for (long long i = 1; i < strCnt; i++) {
        currentTimeRange += time[i] - time[i - 1];
        reqCnt++;

        while (currentTimeRange > timeRange && left + 1 < strCnt) {
            currentTimeRange -= time[left + 1] - time[left];
            left++;
            reqCnt--;
        }

        if (reqCnt > maxReqCnt) {
            firstMark = time[left];
            secondMark = time[i];
            maxReqCnt = reqCnt;
            start = left;
            end = i;
        }
    }

    fclose(file);
    free(time);

    printf("\n%lld seconds time range (%lld to %lld).\nMaximum requests for this time range: %lld (%lld - %lld lines).\n", timeRange, firstMark, secondMark, maxReqCnt, start + 1, end + 1);
    printf("\n%lld errors total.\n", errorsCnt);

    if (errorsCnt) {
        for (long long i = 0; i < errorsCnt; i++) {
            printf("%s\n", res[i]);
        }
    }
    return 0;
}