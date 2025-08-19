#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;
#endif /* __PROGTEST__ */

vector<size_t> getFibonacci(){
    vector<size_t> res;
    int newVal = 1, oldVal = 1;
    for(size_t i = 0; i < 29; ++i){
        int temp = newVal;
        res.push_back(newVal);
        newVal += oldVal;
        oldVal = temp;
    }
    return res;
}

void reverseStr(string & src) {
    for(size_t i = 0; i < src.length() / 2; ++i){
        swap(src[i],src[src.length() - i - 1]);
    }
}

int powTo(int num,int n){
    int res = 1;
    for(int i = 0; i < n; ++i){
        res *= num;
    }
    return res;
}

class Pomohator{
public:
    static vector<size_t> fibKa;
    static string getFibStr(size_t& number){
        string res;
        vector<int> indx;
        number += 1;
        bool flag = false;
        for(int i = (int)fibKa.size() - 1; 0 <= i && number > 0; --i){
            if(number >= fibKa[i]){
                number -= fibKa[i];
                indx.insert(indx.begin(),i);
                flag = true;
            } else if(flag){
                indx.insert(indx.begin(),-1);
            }
        }

        if(indx[0] != 0){
            for(int i = 0; i < indx[0]; ++i){
                res += '0';
            }
        }
        for(int i : indx){
            res += (i != -1) ? '1' : '0';
        }
        res += '1';

        return res;
    }

    static size_t binStrToInt(string& src){
        size_t res = 0;
        int n = (int)src.length();
        for (int i = 0; i < n; i++) {
            if (src[i] == '1') {
                res += powTo(2, n - 1 - i);
            }
        }
        return res;
    }

};

vector<size_t> Pomohator::fibKa = getFibonacci();

bool               utf8ToFibonacci                         ( const char      * inFile,
                                                             const char      * outFile )
{
    ofstream writer(outFile,ios::binary);
    ifstream reader(inFile, ios::binary);

    if(reader.fail() || writer.fail() || !reader.is_open() || !writer.is_open()){
        writer.close();
        reader.close();
        return false;
    }
    auto fib = getFibonacci();
    char input;
    string bits;
    size_t mainNumber;

    while(reader.get(input)){
        mainNumber = (unsigned char)input;
        if((mainNumber & 0xF0) == 0xF0){
            if(((mainNumber >> 3) & 1) != 0){
                writer.close();
                reader.close();
                return false;
            }
            int flag = 0;
            size_t byte1 = mainNumber;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte2 = (unsigned char)input;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte3 = (unsigned char)input;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte4 = (unsigned char)input;
            if(flag != 3 || ((byte2 >> 6) != 0x2) || ((byte3 >> 6) != 0x2) || ((byte4 >> 6) != 0x2)){
                writer.close();
                reader.close();
                return false;
            }
            byte4 = (byte4 & 0x3F);
            byte3 = ((byte3 & 0x3F) << 6);
            byte2 = ((byte2 & 0x3F) << 12);
            byte1 = ((byte1 & 0x7) << 18);
            mainNumber = (byte1 | byte2 | byte3 | byte4);
        }
        else if((mainNumber & 0xE0) == 0xE0){
          if(((mainNumber >> 4) & 1) != 0){
                writer.close();
                reader.close();
                return false;
            }
            int flag = 0;
            size_t byte1 = mainNumber;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte2 = (unsigned char)input;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte3 = (unsigned char)input;
            if(flag != 2 || ((byte2 >> 6) != 0x2) || ((byte3 >> 6) != 0x2)){
                writer.close();
                reader.close();
                return false;
            }
            byte3 = (byte3 & 0x3F);
            byte2 = ((byte2 & 0x3F) << 6);
            byte1 = ((byte1 & 0xF) << 12);
            mainNumber = (byte1 | byte2 | byte3);
        }
        else if((mainNumber & 0xC0) == 0xC0){
           if(((mainNumber >> 5) & 1) != 0){
                writer.close();
                reader.close();
                 return false;
            }
            int flag = 0;
            size_t byte1 = mainNumber;
            if(reader.get(input)){
                ++flag;
            }
            size_t byte2 = (unsigned char)input;
            if(flag != 1 || ((byte2 >> 6) != 0x2)){
                writer.close();
                reader.close();
                return false;
            }
            byte2 = (byte2 & 0x3F);
            byte1 = ((byte1 & 0x1F) << 6);
            mainNumber = (byte1 | byte2);
        } else if((mainNumber >> 7) != 0){
            writer.close();
            reader.close();
            return false;
        }
        if(mainNumber >  0x10ffff){
            writer.close();
            reader.close();
            return false;
        }
        
        string line = Pomohator::getFibStr(mainNumber);
        bits += line;
        while(bits.length() >= 8){
            string temp = bits;
            bits.clear();
            string subStr = temp.substr(0,8);
            reverseStr(subStr);
            size_t a = Pomohator::binStrToInt(subStr);
            subStr.clear();
            writer.write((const char*)&a,1);
            bits = temp.substr(8,temp.length() - 8);
            temp.clear();
            if(writer.bad()){
                writer.close();
                reader.close();
                return false;
            }
        }
    }
    int n = 8 - (int)bits.length();
    if(n != 8) {
        for (int i = 0; i < n; ++i) {
            bits += '0';
        }

        reverseStr(bits);
        size_t a = Pomohator::binStrToInt(bits);
        writer.write((const char *) &a, 1);
    }
    if(writer.bad()){
            writer.close();
            reader.close();
            return false;
        }

    writer.close();
    reader.close();
    return true;
}

bool               fibonacciToUtf8                         ( const char      * inFile,
                                                             const char      * outFile )
{
    ofstream writer(outFile,ios::binary);
    ifstream reader(inFile, ios::binary);
     if(reader.fail() || writer.fail() || !reader.is_open() || !writer.is_open()){
        writer.close();
        reader.close();
        return false;
    }
    char input;
    string bits;
    string oldBits;
    size_t ind = 0;
    size_t wNum = 0;
    bool isPrev1 = false;

    while(reader.get(input)){
        int number;
        bits = "";
        for(int i = 7; i >= 0; i--) {
            number =  ((input >> i) & 1);
            bits += (number + '0');
        }
        reverseStr(bits);
        cout << bits << " ";
        for(char c: bits) {
            if (c == '1') {
                if (!isPrev1) {
                    wNum += (int)Pomohator::fibKa[ind];
                    isPrev1 = true;
                }
                else {
                    wNum -= 1;
                    if(wNum > 0x10ffff){
                         writer.close();
                        reader.close();
                        return false;
                    }
                    ind = 0;
                    int i = wNum;
                    if(i <= 0x7f){
                        writer.write((const char*)&i, 1);
                    } else if(i <= 0x7ff){
                        int firstByte = (0xC0 | (i >> 6));
                        int secondByte = ((i & 0x3F) | 0x80);
                        writer.write((const char*)&firstByte,1);
                        writer.write((const char*)&secondByte,1);
                    } else if(i <= 0xffff){
                        int firstByte = ((i >> 12) | 0xE0);
                        int secondByte = (((i & 0xFC0) >> 6) | 0x80);
                        int thirdByte = ((i & 0x3F) | 0x80);
                        writer.write((const char*)&firstByte,1);
                        writer.write((const char*)&secondByte,1);
                        writer.write((const char*)&thirdByte,1);
                    } else if(i <= 0x10ffff){
                        int firstByte = (((i >> 18) & 0x7) | 0xF0);
                        int secondByte = (((i & 0x3F000) >> 12) | 0x80);
                        int thirdByte = (((i & 0xFC0) >> 6) | 0x80);
                        int fourthByte = ((i & 0x3F) | 0x80);
                        writer.write((const char*)&firstByte,1);
                        writer.write((const char*)&secondByte,1);
                        writer.write((const char*)&thirdByte,1);
                        writer.write((const char*)&fourthByte,1);
                    }
                    if(writer.bad()){
                        writer.close();
                        reader.close();
                        return false;
                    }
                    wNum = 0;
                    isPrev1 = false;
                    continue;
                }
            } else {
                isPrev1 = false;
            }
            ++ind;
        }

        if(reader.peek() != EOF){
            oldBits = bits;   
        }
    }

    if(bits == "00000000"){
        writer.close();
        reader.close();
        return false;
    }

    for(int i = (int)bits.length() - 1; i >= 1; --i){
        if((bits[i] == '1') && (bits[i-1] == '1')){
            break; 
        } else if(bits[i] == '1' && bits[i - 1] != '1'){
            writer.close();
            reader.close();
            return false;
        }
    }



    writer.close();
    reader.close();

    return true;
}
