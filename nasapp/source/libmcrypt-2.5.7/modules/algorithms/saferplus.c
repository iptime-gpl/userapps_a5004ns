/* This is an independent implementation of the encryption algorithm:   */
/*                                                                      */
/*         SAFER+ by Cylink                                             */
/*                                                                      */
/* which is a candidate algorithm in the Advanced Encryption Standard   */
/* programme of the US National Institute of Standards and Technology.  */
/*                                                                      */
/* Copyright in this implementation is held by Dr B R Gladman but I     */
/* hereby give permission for its free direct or derivative use subject */
/* to acknowledgment of its origin and compliance with any conditions   */
/* that the originators of the algorithm place on its exploitation.     */
/*                                                                      */
/* Dr Brian Gladman (gladman@seven77.demon.co.uk) 14th January 1999     */

/* $Id: saferplus.c,v 1.1.1.1 2012/12/07 04:36:58 thki81 Exp $ */

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */


/* Timing data for SAFER+ (safer.c)

Core timing without I/O endian conversion:

128 bit key:
Key Setup:    4278 cycles
Encrypt:      1722 cycles =    14.9 mbits/sec
Decrypt:      1709 cycles =    15.0 mbits/sec
Mean:         1716 cycles =    14.9 mbits/sec

192 bit key:
Key Setup:    7426 cycles
Encrypt:      2555 cycles =    10.0 mbits/sec
Decrypt:      2530 cycles =    10.1 mbits/sec
Mean:         2543 cycles =    10.1 mbits/sec

256 bit key:
Key Setup:   11313 cycles
Encrypt:      3391 cycles =     7.5 mbits/sec
Decrypt:      3338 cycles =     7.7 mbits/sec
Mean:         3365 cycles =     7.6 mbits/sec

Full timing with I/O endian conversion:

128 bit key:
Key Setup:    3977 cycles
Encrypt:      1751 cycles =    14.6 mbits/sec
Decrypt:      1734 cycles =    14.8 mbits/sec
Mean:         1743 cycles =    14.7 mbits/sec

192 bit key:
Key Setup:    6490 cycles
Encrypt:      2574 cycles =     9.9 mbits/sec
Decrypt:      2549 cycles =    10.0 mbits/sec
Mean:         2562 cycles =    10.0 mbits/sec

256 bit key:
Key Setup:    9487 cycles
Encrypt:      3412 cycles =     7.5 mbits/sec
Decrypt:      3372 cycles =     7.6 mbits/sec
Mean:         3392 cycles =     7.5 mbits/sec

*/

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "saferplus.h"
#include <stdlib.h>

#define _mcrypt_set_key saferplus_LTX__mcrypt_set_key
#define _mcrypt_encrypt saferplus_LTX__mcrypt_encrypt
#define _mcrypt_decrypt saferplus_LTX__mcrypt_decrypt
#define _mcrypt_get_size saferplus_LTX__mcrypt_get_size
#define _mcrypt_get_block_size saferplus_LTX__mcrypt_get_block_size
#define _is_block_algorithm saferplus_LTX__is_block_algorithm
#define _mcrypt_get_key_size saferplus_LTX__mcrypt_get_key_size
#define _mcrypt_get_supported_key_sizes saferplus_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name saferplus_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test saferplus_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version saferplus_LTX__mcrypt_algorithm_version

byte safer_expf[256] =
    { 1, 45, 226, 147, 190, 69, 21, 174, 120, 3, 135, 164, 184, 56, 207,
	63,
	8, 103, 9, 148, 235, 38, 168, 107, 189, 24, 52, 27, 187, 191, 114,
	247,
	64, 53, 72, 156, 81, 47, 59, 85, 227, 192, 159, 216, 211, 243, 141,
	177,
	255, 167, 62, 220, 134, 119, 215, 166, 17, 251, 244, 186, 146, 145,
	100, 131,
	241, 51, 239, 218, 44, 181, 178, 43, 136, 209, 153, 203, 140, 132,
	29, 20,
	129, 151, 113, 202, 95, 163, 139, 87, 60, 130, 196, 82, 92, 28,
	232, 160,
	4, 180, 133, 74, 246, 19, 84, 182, 223, 12, 26, 142, 222, 224, 57,
	252,
	32, 155, 36, 78, 169, 152, 158, 171, 242, 96, 208, 108, 234, 250,
	199, 217,
	0, 212, 31, 110, 67, 188, 236, 83, 137, 254, 122, 93, 73, 201, 50,
	194,
	249, 154, 248, 109, 22, 219, 89, 150, 68, 233, 205, 230, 70, 66,
	143, 10,
	193, 204, 185, 101, 176, 210, 198, 172, 30, 65, 98, 41, 46, 14,
	116, 80,
	2, 90, 195, 37, 123, 138, 42, 91, 240, 6, 13, 71, 111, 112, 157,
	126,
	16, 206, 18, 39, 213, 76, 79, 214, 121, 48, 104, 54, 117, 125, 228,
	237,
	128, 106, 144, 55, 162, 94, 118, 170, 197, 127, 61, 175, 165, 229,
	25, 97,
	253, 77, 124, 183, 11, 238, 173, 75, 34, 245, 231, 115, 35, 33,
	200, 5,
	225, 102, 221, 179, 88, 105, 99, 86, 15, 161, 49, 149, 23, 7, 58,
	40
};

byte safer_logf[512] = {
	128, 0, 176, 9, 96, 239, 185, 253, 16, 18, 159, 228, 105, 186, 173,
	248,
	192, 56, 194, 101, 79, 6, 148, 252, 25, 222, 106, 27, 93, 78, 168,
	130,
	112, 237, 232, 236, 114, 179, 21, 195, 255, 171, 182, 71, 68, 1,
	172, 37,
	201, 250, 142, 65, 26, 33, 203, 211, 13, 110, 254, 38, 88, 218, 50,
	15,
	32, 169, 157, 132, 152, 5, 156, 187, 34, 140, 99, 231, 197, 225,
	115, 198,
	175, 36, 91, 135, 102, 39, 247, 87, 244, 150, 177, 183, 92, 139,
	213, 84,
	121, 223, 170, 246, 62, 163, 241, 17, 202, 245, 209, 23, 123, 147,
	131, 188,
	189, 82, 30, 235, 174, 204, 214, 53, 8, 200, 138, 180, 226, 205,
	191, 217,
	208, 80, 89, 63, 77, 98, 52, 10, 72, 136, 181, 86, 76, 46, 107,
	158,
	210, 61, 60, 3, 19, 251, 151, 81, 117, 74, 145, 113, 35, 190, 118,
	42,
	95, 249, 212, 85, 11, 220, 55, 49, 22, 116, 215, 119, 167, 230, 7,
	219,
	164, 47, 70, 243, 97, 69, 103, 227, 12, 162, 59, 28, 133, 24, 4,
	29,
	41, 160, 143, 178, 90, 216, 166, 126, 238, 141, 83, 75, 161, 154,
	193, 14,
	122, 73, 165, 44, 129, 196, 199, 54, 43, 127, 67, 149, 51, 242,
	108, 104,
	109, 240, 2, 40, 206, 221, 155, 234, 94, 153, 124, 20, 134, 207,
	229, 66,
	184, 64, 120, 45, 58, 233, 100, 31, 146, 144, 125, 57, 111, 224,
	137, 48,

	128, 0, 176, 9, 96, 239, 185, 253, 16, 18, 159, 228, 105, 186, 173,
	248,
	192, 56, 194, 101, 79, 6, 148, 252, 25, 222, 106, 27, 93, 78, 168,
	130,
	112, 237, 232, 236, 114, 179, 21, 195, 255, 171, 182, 71, 68, 1,
	172, 37,
	201, 250, 142, 65, 26, 33, 203, 211, 13, 110, 254, 38, 88, 218, 50,
	15,
	32, 169, 157, 132, 152, 5, 156, 187, 34, 140, 99, 231, 197, 225,
	115, 198,
	175, 36, 91, 135, 102, 39, 247, 87, 244, 150, 177, 183, 92, 139,
	213, 84,
	121, 223, 170, 246, 62, 163, 241, 17, 202, 245, 209, 23, 123, 147,
	131, 188,
	189, 82, 30, 235, 174, 204, 214, 53, 8, 200, 138, 180, 226, 205,
	191, 217,
	208, 80, 89, 63, 77, 98, 52, 10, 72, 136, 181, 86, 76, 46, 107,
	158,
	210, 61, 60, 3, 19, 251, 151, 81, 117, 74, 145, 113, 35, 190, 118,
	42,
	95, 249, 212, 85, 11, 220, 55, 49, 22, 116, 215, 119, 167, 230, 7,
	219,
	164, 47, 70, 243, 97, 69, 103, 227, 12, 162, 59, 28, 133, 24, 4,
	29,
	41, 160, 143, 178, 90, 216, 166, 126, 238, 141, 83, 75, 161, 154,
	193, 14,
	122, 73, 165, 44, 129, 196, 199, 54, 43, 127, 67, 149, 51, 242,
	108, 104,
	109, 240, 2, 40, 206, 221, 155, 234, 94, 153, 124, 20, 134, 207,
	229, 66,
	184, 64, 120, 45, 58, 233, 100, 31, 146, 144, 125, 57, 111, 224,
	137, 48
};


/* byte  l_key[33 * 16]; */
/* word32  k_bytes; */

WIN32DLL_DEFINE
    int _mcrypt_set_key(SPI * sp_key, const word32 * in_key,
			const word32 key_len)
{
	word32 wlk[9]; /* 36 bytes */
	byte *lk = (void*)wlk, by; /* lk should have 33 bytes */
	word32 i, j, k, l, m, *lkp = wlk;

	memset( wlk, 0, sizeof(wlk));

	for (i = 0; i < key_len / sizeof(word32); ++i) {
		lkp[i] = in_key[(key_len / sizeof(word32)) - i - 1];
	}

	sp_key->k_bytes = key_len;
	lk[sp_key->k_bytes] = 0;

	for (i = 0; i < sp_key->k_bytes; ++i) {
		lk[sp_key->k_bytes] ^= lk[i];
		sp_key->l_key[i] = lk[i];
	}

	for (i = 0; i < sp_key->k_bytes; ++i) {
		for (j = 0; j <= sp_key->k_bytes; ++j) {
			by = lk[j];
			lk[j] = by << 3 | by >> 5;
		}

		k = 17 * i + 35;
		l = 16 * i + 16;
		m = i + 1;

		if (i < 16) {
			for (j = 0; j < 16; ++j) {
				sp_key->l_key[l + j] =
				    lk[m] +
				    safer_expf[safer_expf[(k + j) & 255]];

				m = (m == sp_key->k_bytes ? 0 : m + 1);
			}
		} else {
			for (j = 0; j < 16; ++j) {
				sp_key->l_key[l + j] =
				    lk[m] + safer_expf[(k + j) & 255];

				m = (m == sp_key->k_bytes ? 0 : m + 1);
			}
		}
	}
	return 0;
}

void do_fr(byte x[16], byte * kp)
{
	byte t;

	x[0] = safer_expf[x[0] ^ kp[0]] + kp[16];
	x[1] = safer_logf[x[1] + kp[1]] ^ kp[17];
	x[2] = safer_logf[x[2] + kp[2]] ^ kp[18];
	x[3] = safer_expf[x[3] ^ kp[3]] + kp[19];

	x[4] = safer_expf[x[4] ^ kp[4]] + kp[20];
	x[5] = safer_logf[x[5] + kp[5]] ^ kp[21];
	x[6] = safer_logf[x[6] + kp[6]] ^ kp[22];
	x[7] = safer_expf[x[7] ^ kp[7]] + kp[23];

	x[8] = safer_expf[x[8] ^ kp[8]] + kp[24];
	x[9] = safer_logf[x[9] + kp[9]] ^ kp[25];
	x[10] = safer_logf[x[10] + kp[10]] ^ kp[26];
	x[11] = safer_expf[x[11] ^ kp[11]] + kp[27];

	x[12] = safer_expf[x[12] ^ kp[12]] + kp[28];
	x[13] = safer_logf[x[13] + kp[13]] ^ kp[29];
	x[14] = safer_logf[x[14] + kp[14]] ^ kp[30];
	x[15] = safer_expf[x[15] ^ kp[15]] + kp[31];

	x[1] += x[0];
	x[0] += x[1];
	x[3] += x[2];
	x[2] += x[3];
	x[5] += x[4];
	x[4] += x[5];
	x[7] += x[6];
	x[6] += x[7];
	x[9] += x[8];
	x[8] += x[9];
	x[11] += x[10];
	x[10] += x[11];
	x[13] += x[12];
	x[12] += x[13];
	x[15] += x[14];
	x[14] += x[15];

	x[7] += x[0];
	x[0] += x[7];
	x[1] += x[2];
	x[2] += x[1];
	x[3] += x[4];
	x[4] += x[3];
	x[5] += x[6];
	x[6] += x[5];
	x[11] += x[8];
	x[8] += x[11];
	x[9] += x[10];
	x[10] += x[9];
	x[15] += x[12];
	x[12] += x[15];
	x[13] += x[14];
	x[14] += x[13];

	x[3] += x[0];
	x[0] += x[3];
	x[15] += x[2];
	x[2] += x[15];
	x[7] += x[4];
	x[4] += x[7];
	x[1] += x[6];
	x[6] += x[1];
	x[5] += x[8];
	x[8] += x[5];
	x[13] += x[10];
	x[10] += x[13];
	x[11] += x[12];
	x[12] += x[11];
	x[9] += x[14];
	x[14] += x[9];

	x[13] += x[0];
	x[0] += x[13];
	x[5] += x[2];
	x[2] += x[5];
	x[9] += x[4];
	x[4] += x[9];
	x[11] += x[6];
	x[6] += x[11];
	x[15] += x[8];
	x[8] += x[15];
	x[1] += x[10];
	x[10] += x[1];
	x[3] += x[12];
	x[12] += x[3];
	x[7] += x[14];
	x[14] += x[7];

	t = x[0];
	x[0] = x[14];
	x[14] = x[12];
	x[12] = x[10];
	x[10] = x[2];
	x[2] = x[8];
	x[8] = x[4];
	x[4] = t;

	t = x[1];
	x[1] = x[7];
	x[7] = x[11];
	x[11] = x[5];
	x[5] = x[13];
	x[13] = t;

	t = x[15];
	x[15] = x[3];
	x[3] = t;
}

void do_ir(byte x[16], byte * kp)
{
	byte t;

	t = x[3];
	x[3] = x[15];
	x[15] = t;

	t = x[13];
	x[13] = x[5];
	x[5] = x[11];
	x[11] = x[7];
	x[7] = x[1];
	x[1] = t;

	t = x[4];
	x[4] = x[8];
	x[8] = x[2];
	x[2] = x[10];
	x[10] = x[12];
	x[12] = x[14];
	x[14] = x[0];
	x[0] = t;

	x[14] -= x[7];
	x[7] -= x[14];
	x[12] -= x[3];
	x[3] -= x[12];
	x[10] -= x[1];
	x[1] -= x[10];
	x[8] -= x[15];
	x[15] -= x[8];
	x[6] -= x[11];
	x[11] -= x[6];
	x[4] -= x[9];
	x[9] -= x[4];
	x[2] -= x[5];
	x[5] -= x[2];
	x[0] -= x[13];
	x[13] -= x[0];

	x[14] -= x[9];
	x[9] -= x[14];
	x[12] -= x[11];
	x[11] -= x[12];
	x[10] -= x[13];
	x[13] -= x[10];
	x[8] -= x[5];
	x[5] -= x[8];
	x[6] -= x[1];
	x[1] -= x[6];
	x[4] -= x[7];
	x[7] -= x[4];
	x[2] -= x[15];
	x[15] -= x[2];
	x[0] -= x[3];
	x[3] -= x[0];

	x[14] -= x[13];
	x[13] -= x[14];
	x[12] -= x[15];
	x[15] -= x[12];
	x[10] -= x[9];
	x[9] -= x[10];
	x[8] -= x[11];
	x[11] -= x[8];
	x[6] -= x[5];
	x[5] -= x[6];
	x[4] -= x[3];
	x[3] -= x[4];
	x[2] -= x[1];
	x[1] -= x[2];
	x[0] -= x[7];
	x[7] -= x[0];

	x[14] -= x[15];
	x[15] -= x[14];
	x[12] -= x[13];
	x[13] -= x[12];
	x[10] -= x[11];
	x[11] -= x[10];
	x[8] -= x[9];
	x[9] -= x[8];
	x[6] -= x[7];
	x[7] -= x[6];
	x[4] -= x[5];
	x[5] -= x[4];
	x[2] -= x[3];
	x[3] -= x[2];
	x[0] -= x[1];
	x[1] -= x[0];

	x[0] = safer_logf[x[0] - kp[16] + 256] ^ kp[0];
	x[1] = safer_expf[x[1] ^ kp[17]] - kp[1];
	x[2] = safer_expf[x[2] ^ kp[18]] - kp[2];
	x[3] = safer_logf[x[3] - kp[19] + 256] ^ kp[3];

	x[4] = safer_logf[x[4] - kp[20] + 256] ^ kp[4];
	x[5] = safer_expf[x[5] ^ kp[21]] - kp[5];
	x[6] = safer_expf[x[6] ^ kp[22]] - kp[6];
	x[7] = safer_logf[x[7] - kp[23] + 256] ^ kp[7];

	x[8] = safer_logf[x[8] - kp[24] + 256] ^ kp[8];
	x[9] = safer_expf[x[9] ^ kp[25]] - kp[9];
	x[10] = safer_expf[x[10] ^ kp[26]] - kp[10];
	x[11] = safer_logf[x[11] - kp[27] + 256] ^ kp[11];

	x[12] = safer_logf[x[12] - kp[28] + 256] ^ kp[12];
	x[13] = safer_expf[x[13] ^ kp[29]] - kp[13];
	x[14] = safer_expf[x[14] ^ kp[30]] - kp[14];
	x[15] = safer_logf[x[15] - kp[31] + 256] ^ kp[15];
}

WIN32DLL_DEFINE void _mcrypt_encrypt(SPI * spi, word32 * _blk)
{
	byte *kp;
	word32 __blk[16];
	byte *blk = (void *) __blk;

	__blk[3] = _blk[0];
	__blk[2] = _blk[1];
	__blk[1] = _blk[2];
	__blk[0] = _blk[3];

	do_fr(blk, spi->l_key);
	do_fr(blk, spi->l_key + 32);
	do_fr(blk, spi->l_key + 64);
	do_fr(blk, spi->l_key + 96);
	do_fr(blk, spi->l_key + 128);
	do_fr(blk, spi->l_key + 160);
	do_fr(blk, spi->l_key + 192);
	do_fr(blk, spi->l_key + 224);

	if (spi->k_bytes > 16) {
		do_fr(blk, spi->l_key + 256);
		do_fr(blk, spi->l_key + 288);
		do_fr(blk, spi->l_key + 320);
		do_fr(blk, spi->l_key + 352);
	}

	if (spi->k_bytes > 24) {
		do_fr(blk, spi->l_key + 384);
		do_fr(blk, spi->l_key + 416);
		do_fr(blk, spi->l_key + 448);
		do_fr(blk, spi->l_key + 480);
	}

	kp = spi->l_key + 16 * spi->k_bytes;

	blk[0] ^= kp[0];
	blk[1] += kp[1];
	blk[2] += kp[2];
	blk[3] ^= kp[3];
	blk[4] ^= kp[4];
	blk[5] += kp[5];
	blk[6] += kp[6];
	blk[7] ^= kp[7];
	blk[8] ^= kp[8];
	blk[9] += kp[9];
	blk[10] += kp[10];
	blk[11] ^= kp[11];
	blk[12] ^= kp[12];
	blk[13] += kp[13];
	blk[14] += kp[14];
	blk[15] ^= kp[15];

	_blk[3] = (__blk[0]);
	_blk[2] = (__blk[1]);
	_blk[1] = (__blk[2]);
	_blk[0] = (__blk[3]);
}



WIN32DLL_DEFINE void _mcrypt_decrypt(SPI * spi, word32 * _blk)
{
	byte *kp;
	word32 __blk[16];
	byte *blk = (void *) __blk;

	__blk[3] = _blk[0];
	__blk[2] = _blk[1];
	__blk[1] = _blk[2];
	__blk[0] = _blk[3];

	kp = spi->l_key + 16 * spi->k_bytes;

	blk[0] ^= kp[0];
	blk[1] -= kp[1];
	blk[2] -= kp[2];
	blk[3] ^= kp[3];
	blk[4] ^= kp[4];
	blk[5] -= kp[5];
	blk[6] -= kp[6];
	blk[7] ^= kp[7];
	blk[8] ^= kp[8];
	blk[9] -= kp[9];
	blk[10] -= kp[10];
	blk[11] ^= kp[11];
	blk[12] ^= kp[12];
	blk[13] -= kp[13];
	blk[14] -= kp[14];
	blk[15] ^= kp[15];

	if (spi->k_bytes > 24) {
		do_ir(blk, spi->l_key + 480);
		do_ir(blk, spi->l_key + 448);
		do_ir(blk, spi->l_key + 416);
		do_ir(blk, spi->l_key + 384);
	}

	if (spi->k_bytes > 16) {
		do_ir(blk, spi->l_key + 352);
		do_ir(blk, spi->l_key + 320);
		do_ir(blk, spi->l_key + 288);
		do_ir(blk, spi->l_key + 256);
	}

	do_ir(blk, spi->l_key + 224);
	do_ir(blk, spi->l_key + 192);
	do_ir(blk, spi->l_key + 160);
	do_ir(blk, spi->l_key + 128);
	do_ir(blk, spi->l_key + 96);
	do_ir(blk, spi->l_key + 64);
	do_ir(blk, spi->l_key + 32);
	do_ir(blk, spi->l_key);

	_blk[3] = (__blk[0]);
	_blk[2] = (__blk[1]);
	_blk[1] = (__blk[2]);
	_blk[0] = (__blk[3]);
}



WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(SPI);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 16;
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 32;
}
static const int key_sizes[] = { 16,24,32 };
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = sizeof(key_sizes)/sizeof(int);
	return key_sizes;

}
WIN32DLL_DEFINE char *_mcrypt_get_algorithms_name()
{
return "Safer+";
}

#define CIPHER "97fa76704bf6b578549f65c6f75b228b"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	char *keyword;
	unsigned char plaintext[16];
	unsigned char ciphertext[16];
	int blocksize = _mcrypt_get_block_size(), j;
	void *key;
	unsigned char cipher_tmp[200];

	keyword = calloc(1, _mcrypt_get_key_size());
	if (keyword == NULL)
		return -1;

	for (j = 0; j < _mcrypt_get_key_size(); j++) {
		keyword[j] = ((j * 2 + 10) % 256);
	}

	for (j = 0; j < blocksize; j++) {
		plaintext[j] = j % 256;
	}
	key = malloc(_mcrypt_get_size());
	if (key == NULL)
		return -1;

	memcpy(ciphertext, plaintext, blocksize);

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size());
	free(keyword);

	_mcrypt_encrypt(key, (void *) ciphertext);

	for (j = 0; j < blocksize; j++) {
		sprintf(&((char *) cipher_tmp)[2 * j], "%.2x",
			ciphertext[j]);
	}

	if (strcmp((char *) cipher_tmp, CIPHER) != 0) {
		printf("failed compatibility\n");
		printf("Expected: %s\nGot: %s\n", CIPHER,
		       (char *) cipher_tmp);
		free(key);
		return -1;
	}
	_mcrypt_decrypt(key, (void *) ciphertext);
	free(key);

	if (strcmp(ciphertext, plaintext) != 0) {
		printf("failed internally\n");
		return -1;
	}

	return 0;
}

WIN32DLL_DEFINE word32 _mcrypt_algorithm_version()
{
	return 20010801;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif