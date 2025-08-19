#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

struct crypto_config
{
	const char * m_crypto_function;
	std::unique_ptr<uint8_t[]> m_key;
	std::unique_ptr<uint8_t[]> m_IV;
	size_t m_key_len;
	size_t m_IV_len;
};

#endif /* _PROGTEST_ */

class CCtx{
public:
	CCtx() { _ctx = EVP_CIPHER_CTX_new(); }
	~CCtx() { EVP_CIPHER_CTX_free(_ctx); }
	EVP_CIPHER_CTX * _ctx;
};

bool cipher_data(const std::string & in_filename, const std::string & out_filename, crypto_config & config, bool encrypt)
{	
	ifstream reader(in_filename, ios::binary);
	ofstream writer(out_filename, ios::binary);
	if(!reader || !writer) 
		return false;
	
	char header[18];
	reader.read(header, 18);
	if(reader.gcount() != 18 ||  reader.fail()) 
		return false;

	writer.write(header, 18);
	if(writer.fail())
		return false;
	
	OpenSSL_add_all_algorithms();

	const EVP_CIPHER * cipher = EVP_get_cipherbyname(config.m_crypto_function);
	if(!cipher)
		return false;

	CCtx ctx;
	if(!ctx._ctx) 
		return false;
	
	size_t keyLen = EVP_CIPHER_key_length(cipher);
	size_t ivLen = EVP_CIPHER_iv_length(cipher);

	if(!config.m_key.get() || config.m_key_len < keyLen) {
		uint8_t newKey[EVP_MAX_KEY_LENGTH];
		if(!encrypt || !RAND_bytes(newKey, keyLen))  
			return false;
		
		config.m_key = make_unique<uint8_t[]>(keyLen);
		config.m_key_len = keyLen;
		memcpy(config.m_key.get(), newKey, keyLen);
	}
	if((!config.m_IV.get() || config.m_IV_len < ivLen) && ivLen != 0) {
		uint8_t newIV[EVP_MAX_IV_LENGTH];
		if(!encrypt || !RAND_bytes(newIV, ivLen))  
			return false;
		
		config.m_IV = make_unique<uint8_t[]>(ivLen);
		config.m_IV_len = ivLen;
		memcpy(config.m_IV.get(), newIV, ivLen);
	}

	if(!EVP_CipherInit_ex(ctx._ctx, cipher, NULL, config.m_key.get(), config.m_IV.get(), encrypt))
		return false;
	
	const int MAX_BUFFER_SIZE = 512; 
	uint8_t bufferRead[MAX_BUFFER_SIZE];
	uint8_t bufferWrite[MAX_BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
	int countRead, countWrite;
	
	while(true) {
		reader.read((char*)bufferRead, MAX_BUFFER_SIZE);
		if(reader.eof()) {
			countRead = reader.gcount();
			if(countRead > 0) {
				if(!EVP_CipherUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
					return false;

				writer.write((char*)bufferWrite, countWrite);
				if(writer.fail()) 
					return false;
			}
			break;
		}

		if(reader.fail())
			return false;
		
		countRead = reader.gcount();
		if(!EVP_CipherUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
			return false;
		
		writer.write((char*)bufferWrite, countWrite);
		if(writer.fail()) 
			return false;
	}

	if(!EVP_CipherFinal(ctx._ctx, bufferWrite, &countWrite))
		return false;

	writer.write((char*)bufferWrite, countWrite);
	if(writer.fail()) 
		return false;

	return true;
}

bool encrypt_data ( const std::string & in_filename, const std::string & out_filename, crypto_config & config )
{
	return cipher_data(in_filename, out_filename, config, true);
}

bool decrypt_data ( const std::string & in_filename, const std::string & out_filename, crypto_config & config )
{
	return cipher_data(in_filename, out_filename, config, false);
}


#ifndef __PROGTEST__

bool compare_files ( const char * name1, const char * name2 )
{
	ifstream file1(name1, ios::binary);
    ifstream file2(name2, ios::binary);

    if (!file1 || !file2)
        return false;

    string line1, line2;
    while (true) {
		file1 >> line1;
		file2 >> line2;
		if(file1.eof() || file2.eof()) {
			break;
		}
        if(line1 != line2) {
            return false;
        }
    }

    return (file1.eof() && file2.eof());
}

int main ( void )
{
	crypto_config config {nullptr, nullptr, nullptr, 0, 0};

	// ECB mode
	config.m_crypto_function = "AES-128-ECB";
	config.m_key = std::make_unique<uint8_t[]>(16);
 	memset(config.m_key.get(), 0, 16);
	config.m_key_len = 16;

	assert( encrypt_data  ("homer-simpson.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson_enc_ecb.TGA") );

	assert( decrypt_data  ("homer-simpson_enc_ecb.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson.TGA") );

	assert( encrypt_data  ("UCM8.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8_enc_ecb.TGA") );

	assert( decrypt_data  ("UCM8_enc_ecb.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8.TGA") );

	assert( encrypt_data  ("image_1.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_1_enc_ecb.TGA") );

	assert( encrypt_data  ("image_2.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_2_enc_ecb.TGA") );

	assert( decrypt_data ("image_3_enc_ecb.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_3_dec_ecb.TGA") );

	assert( decrypt_data ("image_4_enc_ecb.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_4_dec_ecb.TGA") );

	// CBC mode
	config.m_crypto_function = "AES-128-CBC";
	config.m_IV = std::make_unique<uint8_t[]>(16);
	config.m_IV_len = 16;
	memset(config.m_IV.get(), 0, 16);

	assert( encrypt_data  ("UCM8.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8_enc_cbc.TGA") );

	assert( decrypt_data  ("UCM8_enc_cbc.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8.TGA") );

	assert( encrypt_data  ("homer-simpson.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson_enc_cbc.TGA") );

	assert( decrypt_data  ("homer-simpson_enc_cbc.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson.TGA") );

	assert( encrypt_data  ("image_1.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_5_enc_cbc.TGA") );

	assert( encrypt_data  ("image_2.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_6_enc_cbc.TGA") );

	assert( decrypt_data ("image_7_enc_cbc.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_7_dec_cbc.TGA") );

	assert( decrypt_data ("image_8_enc_cbc.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_8_dec_cbc.TGA") );
	return 0;
}

#endif /* _PROGTEST_ */
