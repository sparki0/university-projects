#ifndef __PROGTEST__
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

#endif /* __PROGTEST__ */

bool myCheck(int bits, unsigned char hash[], int len) {
    for(int i = 0; (i < len) && bits ; ++i) {
        for (int j = 7; j >= 0; --j) {
            if (! ((hash[i] >> j) & 1)) {
                --bits;
                if(!bits) 
                    return true;
                continue;
            }
            return false;
        }
    }
    return true;
}

std::string charToHex(unsigned char* ptr, int len) {
    string res = "";
    char hex[3];
    for(int i = 0; i < len; ++i) {
        sprintf(hex, "%02x" ,ptr[i]);
        res += hex;
    }

    return res;
}

int findHash (int numberZeroBits, string & outputMessage, string & outputHash) {
    if(numberZeroBits < 0 || numberZeroBits > 512) {
        return 0;
    }

    vector<unsigned char> text(64,0);
    char hashFunction[] = "sha512";
    EVP_MD_CTX * ctx; 
    const EVP_MD * type; 
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length; 

    OpenSSL_add_all_digests();
    type = EVP_get_digestbyname(hashFunction);

    if (!type) {
        printf("Hash %s neexistuje.\n", hashFunction);
        return 0;
    }

    ctx = EVP_MD_CTX_new(); 
    if (ctx == NULL)
        return 0;
    while(true) {
        if (!EVP_DigestInit_ex(ctx, type, NULL)) 
            return 0;
        if (!EVP_DigestUpdate(ctx, text.data() ,  text.size())) 
            return 0;
        if (!EVP_DigestFinal_ex(ctx, hash, &length))
            return 0;
        
        if(myCheck(numberZeroBits, hash, length))
            break;
        
        for(size_t i = 0; i < text.size(); ++i){
            text[i] = hash[i];
        }
    }

    EVP_MD_CTX_free(ctx); 
    outputMessage = charToHex(text.data(), text.size());
    outputHash = charToHex(hash,length);

    return 1;
}

int findHashEx (int numberZeroBits, string & outputMessage, string & outputHash, string_view hashType) {
    /* TODO or use dummy implementation */
    return 1;
}

#ifndef __PROGTEST__

int checkHash(int bits, const string & hash) {
    return 0;
}

int main (void) {
    string hash, message;
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */

