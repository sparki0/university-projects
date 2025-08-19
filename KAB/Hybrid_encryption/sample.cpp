#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pem.h>

using namespace std;

#endif /* __PROGTEST__ */

class CCtx{
public:
	CCtx() { _ctx = EVP_CIPHER_CTX_new(); }
	~CCtx() { EVP_CIPHER_CTX_free(_ctx); }
	EVP_CIPHER_CTX * _ctx;
};

class CKey {
public:
    CKey(FILE * const file, bool flag) { 
        _pK = flag ?  PEM_read_PrivateKey(file, NULL, NULL, NULL) : PEM_read_PUBKEY(file, NULL, NULL, NULL);  
    } 
    ~CKey() { EVP_PKEY_free(_pK); }
    EVP_PKEY * _pK;
};

bool sealWork(string_view inFile, string_view outFile, string_view publicKeyFile, string_view symmetricCipher) {
    ifstream reader(inFile.data(), ios::binary);
    ofstream writer(outFile.data(), ios::binary);
    FILE * pubKeyFile = fopen(publicKeyFile.data(), "r");
    if(!pubKeyFile) 
        return false;

    if(!reader || !writer ) {
        fclose(pubKeyFile);
        return false;
    }

    OpenSSL_add_all_algorithms();
    
    CKey publicKey(pubKeyFile, false);
    fclose(pubKeyFile);
    if(!publicKey._pK)
        return false;

    const EVP_CIPHER * cipher = EVP_get_cipherbyname(symmetricCipher.data());
    CCtx ctx;

    if(!cipher || !ctx._ctx) 
        return false;


    auto ek = make_unique<uint8_t[]>(EVP_PKEY_size(publicKey._pK));
    int ekLen;
    int ivLen = (EVP_CIPHER_iv_length(cipher));
    auto iv = make_unique<uint8_t[]>(ivLen);
    EVP_PKEY * publicKeys[] = { publicKey._pK };
    uint8_t * ekArr[] = { ek.get() };
    if(!EVP_SealInit(ctx._ctx, cipher, ekArr, &ekLen, iv.get(), publicKeys, 1)) 
        return false;
    

    int nid = EVP_CIPHER_nid(cipher);
    writer.write(reinterpret_cast<char*>(&nid), 4);
    if(writer.fail()) 
        return false;

    writer.write(reinterpret_cast<char*>(&ekLen), 4);
    if(writer.fail()) 
        return false;

    writer.write((char*)ek.get(), ekLen);
    if(writer.fail()) 
        return false;

    if(ivLen) {
        writer.write((char*)iv.get(), ivLen);
        if(writer.fail()) {
            return false;            
        }
    }
    
    const int MAX_BUFFER_SIZE = 512; 
	uint8_t bufferRead[MAX_BUFFER_SIZE];
	uint8_t bufferWrite[MAX_BUFFER_SIZE + EVP_MAX_BLOCK_LENGTH];
	int countRead, countWrite;
	
	while(true) {
		reader.read((char*)bufferRead, MAX_BUFFER_SIZE);
		if(reader.eof()) {
			countRead = reader.gcount();
			if(countRead > 0) {
				if(!EVP_SealUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
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
		if(!EVP_SealUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
			return false;
		
		writer.write((char*)bufferWrite, countWrite);
		if(writer.fail()) 
			return false;
	}

	if(!EVP_SealFinal(ctx._ctx, bufferWrite, &countWrite))
		return false;

	writer.write((char*)bufferWrite, countWrite);
	if(writer.fail()) 
		return false;

    return true;
}


bool seal( string_view inFile, string_view outFile, string_view publicKeyFile, string_view symmetricCipher )
{
    if(inFile.empty() || outFile.empty() || publicKeyFile.empty() || symmetricCipher.empty()) 
        return false;

    bool res = sealWork(inFile, outFile, publicKeyFile, symmetricCipher);
    if(!res) {
        remove(outFile.data());
    }

    return res;
}

bool openWork(string_view inFile, string_view outFile, string_view privateKeyFile) {
    ifstream reader(inFile.data(), ios::binary);
    ofstream writer(outFile.data(), ios::binary);
    FILE * priKeyFile = fopen(privateKeyFile.data(), "r");
    if(!priKeyFile) {
        return false;
    }

    if(!reader || !writer) {
        fclose(priKeyFile);
        return false;
    }

    OpenSSL_add_all_algorithms();
    
    CKey privateKey(priKeyFile, true);
    fclose(priKeyFile);
    if(!privateKey._pK)
        return false;

    int nid;
    reader.read(reinterpret_cast<char*>(&nid), 4);
    if(reader.fail() || reader.gcount() != 4) 
        return false;

    const EVP_CIPHER * cipher = EVP_get_cipherbynid(nid);
    CCtx ctx;
    if(!cipher || !ctx._ctx) 
        return false;

    int ekLen;
    reader.read(reinterpret_cast<char*>(&ekLen), 4);
    auto ek = make_unique<uint8_t[]>(ekLen);

    reader.read((char*)ek.get(), ekLen);
    if(reader.fail() || reader.gcount() != ekLen) 
        return false;

    int ivLen = (EVP_CIPHER_iv_length(cipher));
    auto iv = make_unique<uint8_t[]>(ivLen);

    if(ivLen) {
        reader.read((char*)iv.get(), ivLen);
        if(reader.fail() || reader.gcount() != ivLen) 
            return false;
    }
    
    if(!EVP_OpenInit(ctx._ctx, cipher, ek.get(), ekLen, iv.get(), privateKey._pK)) 
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
				if(!EVP_OpenUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
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
		if(!EVP_OpenUpdate(ctx._ctx, bufferWrite, &countWrite, bufferRead, countRead))
			return false;
		
		writer.write((char*)bufferWrite, countWrite);
		if(writer.fail()) 
			return false;
	}

	if(!EVP_OpenFinal(ctx._ctx, bufferWrite, &countWrite))
		return false;

	writer.write((char*)bufferWrite, countWrite);
	if(writer.fail()) 
		return false;

    return true;
}

bool open( string_view inFile, string_view outFile, string_view privateKeyFile )
{
    if(inFile.empty() || outFile.empty() || privateKeyFile.empty()) 
        return false;
    bool res = openWork(inFile, outFile, privateKeyFile);
    if(!res) {
        remove(outFile.data());
    }
    return res;
}



#ifndef __PROGTEST__

int main ( void )
{
    return 0;
}

#endif /* __PROGTEST__ */

