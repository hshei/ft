#include <openssl/evp.h>
#include <string.h>

void derive_key(const char *password, unsigned char *key) {
    const unsigned char salt[] = "ft_salt_v1";  // fixed salt
    PKCS5_PBKDF2_HMAC(password, strlen(password),
                      salt, sizeof(salt) - 1,
                      100000,           // iterations
                      EVP_sha256(),
                      32, key);         // 32 bytes = 256-bit key
}

int encrypt_chunk(const unsigned char *key, const unsigned char *iv,
                  const unsigned char *plaintext, int pt_len,
                  unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    int len, ct_len;

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, pt_len);
    ct_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ct_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return ct_len;
}

int decrypt_chunk(const unsigned char *key, const unsigned char *iv,
                  const unsigned char *ciphertext, int ct_len,
                  unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;

    int len, pt_len;

    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ct_len);
    pt_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) <= 0) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;   // wrong password or corrupted data
    }
    pt_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return pt_len;
}