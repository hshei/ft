#ifndef CRYPTO_H
#define CRYPTO_H

void derive_key(const char *password, unsigned char *key);
int encrypt_chunk(const unsigned char *key, const unsigned char *iv,
                  const unsigned char *plaintext, int pt_len,
                  unsigned char *ciphertext);
int decrypt_chunk(const unsigned char *key, const unsigned char *iv,
                  const unsigned char *ciphertext, int ct_len,
                  unsigned char *plaintext);

#endif // CRYPTO_H