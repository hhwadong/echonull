#include "../include/security.h"
#include <string.h>

void encryptData(const char *input, char *output, size_t length) {
    const char *key = ENCRYPTION_KEY;
    size_t keyLength = strlen(key);

    for (size_t i = 0; i < length; i++) {
        output[i] = input[i] ^ key[i % keyLength];
    }
}

void decryptData(const char *input, char *output, size_t length) {
    encryptData(input, output, length);
}