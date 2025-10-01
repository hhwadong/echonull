#ifndef SECURITY_H
#define SECURITY_H

#include <stddef.h>

#define ENCRYPTION_KEY "echonull_secure_key_2024"

void encryptData(const char *input, char *output, size_t length);
void decryptData(const char *input, char *output, size_t length);

#endif