#include "ske.h"
#include "prf.h"
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memcpy */
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#ifdef LINUX
#define MMAP_SEQ MAP_PRIVATE|MAP_POPULATE
#else
#define MMAP_SEQ MAP_PRIVATE
#endif

/* NOTE: since we use counter mode, we don't need padding, as the
 * ciphertext length will be the same as that of the plaintext.
 * Here's the message format we'll use for the ciphertext:
 * +------------+--------------------+----------------------------------+
 * | 16 byte IV | C = AES(plaintext) | HMAC(IV|C) (32 bytes for SHA256) |
 * +------------+--------------------+----------------------------------+
 * */

/* we'll use hmac with sha256, which produces 32 byte output */
#define HM_LEN 32
#define KDF_KEY "qVHqkOVJLb7EolR9dsAMVwH1hRCYVx#I"
/* need to make sure KDF is orthogonal to other hash functions, like
 * the one used in the KDF, so we use hmac with a key. */

int ske_keyGen(SKE_KEY* K, unsigned char* entropy, size_t entLen)
{
	/* TODO: write this.  If entropy is given, apply a KDF to it to get
	 * the keys (something like HMAC-SHA512 with KDF_KEY will work).
	 * If entropy is null, just get a random key (you can use the PRF). */
	return 0;
}
size_t ske_getOutputLen(size_t inputLen)
{
	return AES_BLOCK_SIZE + inputLen + HM_LEN;
}
size_t ske_encrypt(unsigned char* outBuf, unsigned char* inBuf, size_t len,
		SKE_KEY* K, unsigned char* IV)
{
	/* TODO: finish writing this.  Look at ctr_example() in aes-example.c
	 * for a hint.  Also, be sure to setup a random IV if none was given.
	 * You can assume outBuf has enough space for the result. */
	return 0; /* TODO: should return number of bytes written, which
	             hopefully matches ske_getOutputLen(...). */
}
size_t ske_encrypt_file(const char* fnout, const char* fnin,
		SKE_KEY* K, unsigned char* IV, size_t offset_out)
{
	/* TODO: write this.  Hint: mmap. */
	/* NOTE: offset determines where to begin writing to the output file.
 	 * set to 0 to erase the file and write it from scratch. */

	// Read from file
	int fd = open(fnin, O_RDONLY);
	struct stat sb;

	if (fstat(fd,&sb) == -1) {
		perror("Could't get input file size.\n");
		return -1;
	}

	char* file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(),0,K->aesKey, IV))
		ERR_print_errors_fp(stderr);
	
	int nWritten;
	unsigned char ct[512];
	// EVP_EncryptUpdate() encrypts inl bytes from the buffer in and 
	// writes the encrypted version to out.
	if (1 != EVP_EncryptUpdate(ctx, ct, &nWritten, (unsigned char*)file_in_memory, sb.st_size))
		ERR_print_errors_fp(stderr);
	
	EVP_CIPHER_CTX_free(ctx);
	munmap(file_in_memory, sb.st_size);
	close(fd);

	// Write to file
	fd = open(fnout, O_RDWR|O_CREAT, S_IRWXU);
	lseek(fd, offset_out, SEEK_CUR);
	write(fd, ct, 512);
	close(fd);

	return 0;
}
size_t ske_decrypt(unsigned char* outBuf, unsigned char* inBuf, size_t len,
		SKE_KEY* K)
{
	/* TODO: write this.  Make sure you check the mac before decypting!
	 * Oh, and also, return -1 if the ciphertext is found invalid.
	 * Otherwise, return the number of bytes written.  See aes-example.c
	 * for how to do basic decryption. */
	return 0;
}
size_t ske_decrypt_file(const char* fnout, const char* fnin,
		SKE_KEY* K, size_t offset_in)
{
	/* TODO: write this. */
	/* NOTE: offset determines where to begin reading the input file. */

	unsigned char pt[512];
	unsigned char iv[16];
	for (size_t i = 0; i < 16; ++i) iv[i] = 1;
	// Read from file
	int fd = open(fnin, O_RDONLY);
	struct stat sb;

	if (fstat(fd,&sb) == -1) {
		perror("Could't get input file size.\n");
		return -1;
	}

	char* file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	int nWritten = 0;
	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), 0, K->aesKey, iv))
		ERR_print_errors_fp(stderr);
	
	if (1 != EVP_DecryptUpdate(ctx,pt,&nWritten,file_in_memory,sb.st_size))
		ERR_print_errors_fp(stderr);
	
	EVP_CIPHER_CTX_free(ctx);
	munmap(file_in_memory, sb.st_size);
	close(fd);
	
	// Write to file
	fd = open(fnout, O_RDWR|O_CREAT, S_IRWXU);
	lseek(fd, offset_in, SEEK_CUR);
	write(fd, pt, 512);
	close(fd);

	return 0;
}
