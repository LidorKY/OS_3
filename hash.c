#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

void hash_file(const char *filename, unsigned char *digest) {
    FILE *fp;
    unsigned char buffer[BUFSIZ];
    size_t bytes_read;
    MD5_CTX context;

    // Open the file
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        exit(1);
    }

    // Initialize the MD5 context
    MD5_Init(&context);

    // Read the file in chunks and update the hash context
    while ((bytes_read = fread(buffer, 1, BUFSIZ, fp)) != 0) {
        MD5_Update(&context, buffer, bytes_read);
    }

    // Finalize the hash and store the result in the provided buffer
    MD5_Final(digest, &context);

    // Close the file
    fclose(fp);
}

int main(int argc, char *argv[]) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    filename = argv[1];

    hash_file(filename, digest);

    printf("Hash of file '%s':\n", filename);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");

    return 0;
}
