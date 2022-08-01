#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define max(a, b) (a > b ? a : b)

const int billion = 1000000000;

typedef struct {
    size_t size;
    int32_t *data;
} uint1024_t;

uint1024_t from_uint(unsigned int num) {
    int length = num >= billion ? 2 : 1;
    uint1024_t uint1024;
    uint1024.data = calloc(length, sizeof(int32_t));
    uint1024.size = length;
    uint1024.data[0] = num % billion;
    if (length > 1) uint1024.data[1] = num / billion;
    return uint1024;
}

void scanf_value(uint1024_t *num) {
    char string[1024];
    scanf("%s", string);
    int numLength = strlen(string);
    int length = numLength % 9 == 0 ? numLength / 9 : numLength / 9 + 1;
    uint1024_t inputValue;
    inputValue.size = length;
    inputValue.data = calloc(length, sizeof(int32_t));
    for (int i = numLength, j = 0; i > 0; i -= 9, j++) {
        string[i] = '\0';
        inputValue.data[j] = i >= 9 ? atoi(string + i - 9) : atoi(string);
    }
    *num = inputValue;
}

void printf_value(uint1024_t num) {
    int length = num.size;
    length != 0 ? printf("%d", num.data[length - 1]) : printf("%d", 0);
    for (int i = length - 2; i >= 0; --i) {
        printf("%09d", num.data[i]);
    }
}

uint1024_t add_op(uint1024_t a, uint1024_t b) {
    uint1024_t sum;
    int toNextDigit;
    int length = max(a.size, b.size);
    sum.data = calloc(length, sizeof(int32_t));
    sum.size = length;
    for (int i = 0; i < length || toNextDigit; ++i) {
        if (i == length) {
            sum.data = realloc(sum.data, (length + 1) * sizeof(int32_t));
            sum.data[length] = 0;
            sum.size++;
        }
        sum.data[i] = a.data[i] + b.data[i] + toNextDigit;
        if (sum.data[i] >= billion) {
            toNextDigit = 1;
            sum.data[i] -= billion;
        } else toNextDigit = 0;
    }
    return sum;
}

uint1024_t subtr_op(uint1024_t a, uint1024_t b) {
    uint1024_t diff;
    int toNextDigit = 0;
    int length = max(a.size, b.size);
    diff.data = calloc(length, sizeof(int32_t));
    diff.size = length;
    for (int i = 0; i < length || toNextDigit; ++i) {
        if (i == length) break;
        diff.data[i] = a.data[i] - b.data[i] - toNextDigit;
        if (diff.data[i] < 0) {
            toNextDigit = 1;
            diff.data[i] += billion;
        } else toNextDigit = 0;
    }
    while (diff.data[diff.size - 1] == 0 && diff.size > 1) diff.size--;
    diff.data = realloc(diff.data, diff.size * sizeof(int32_t));
    return diff;
}

uint1024_t mult_op(uint1024_t a, uint1024_t b) {
    uint1024_t sum;
    int length = a.size + b.size;
    sum.data = calloc(length, sizeof(int32_t));
    sum.size = length;
    for (int i = 0; i < a.size; i++) {
        for (int j = 0, toNextDigit = 0; j < b.size || toNextDigit; ++j) {
            long long tmp = sum.data[i + j] + a.data[i] * 1ll * b.data[j] + toNextDigit;
            sum.data[i + j] = (int) (tmp % billion);
            toNextDigit = (int) (tmp / billion);
        }
    }
    while (sum.data[length - 1] == 0 && length > 1) length--;
    sum.data = realloc(sum.data, length * sizeof(int32_t));
    sum.size = length;
    return sum;
}

int main() {
    uint1024_t a, b;
    printf("%s", "Enter the 1st uint1024_t value.\n");
    scanf_value(&a);
    printf("%s", "Enter the 2nd uint1024_t value.\n");
    scanf_value(&b);
    printf("%s", "Enter the operation type (add, subtract or multiply).\n");
    char cmd[8];
    scanf("%s", cmd);
    if (cmd[0] == 'a') printf_value(add_op(a, b));
    if (cmd[0] == 's') printf_value(subtr_op(a, b));
    if (cmd[0] == 'm') printf_value(mult_op(a, b));
    return 0;
}