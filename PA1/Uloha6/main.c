#include <stdio.h>
#include "ctype.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

typedef struct TShelf{
    int number;
    char * prodName;
} TSHELF;

typedef struct TShopList{
    int listIdx;
    int shelfIdx;
    char * prodName;
} TSHOPLIST;

typedef struct TPointers{
    int shelfIdx;
    char * ptrProd;
    char * ptrShelf;
} TPOINTERS;


char * toLowerStr(char * str);
int cmpList(const void * a, const void * b);

TSHELF * readInputShelf(int * prodCount);
TSHOPLIST * readShopList(int * listCount, int * countCounts);

void freeShelfs(TSHELF * shelfs, int count);
void freeShopList(TSHOPLIST * list,int count);

void findOptList(TSHELF * shelfs,int shelfCount, TSHOPLIST * list, int listCount);

int main() {
    int shelfProdCount;
    TSHELF * shelfs = readInputShelf(&shelfProdCount);
    if(!shelfs){
        printf("Nespravny vstup.\n");
        return 1;
    }
    int listCount;
    int countCounts;
    TSHOPLIST * prodList = readShopList(&listCount,&countCounts);
    if(!prodList){
        freeShelfs(shelfs,shelfProdCount);
        printf("Nespravny vstup.\n");
        return 1;
    }

    findOptList(shelfs,shelfProdCount,prodList,listCount);
    for(int i = 0, idx = 0;i < countCounts; ++i){
        int countPtr = 0;
        int sizePtr = 10;
        TPOINTERS * ptrs = (TPOINTERS*) malloc(sizePtr * sizeof(*ptrs));
        printf("Optimalizovany seznam:\n");
        for(int j = 0; j < listCount; ++j){
            if(i == prodList[j].listIdx){
                if(prodList[j].shelfIdx != -1) {
                    ptrs[countPtr].shelfIdx = shelfs[prodList[j].shelfIdx].number;
                    ptrs[countPtr].ptrShelf = shelfs[prodList[j].shelfIdx].prodName;
                    ptrs[countPtr].ptrProd = prodList[j].prodName;
                    ++countPtr;
                    if(countPtr == sizePtr){
                        sizePtr *= 2;
                        ptrs = (TPOINTERS*) realloc(ptrs,sizePtr * sizeof(*ptrs));
                    }
                }
            }
        }
//        printf("ptr count: %d\n",countPtr);
        qsort(ptrs,countPtr,sizeof(*ptrs),cmpList);
        for(int j = 0; j < countPtr; ++j){
            printf(" %d. %s -> #%d %s\n",idx,ptrs[j].ptrProd, ptrs[j].shelfIdx,ptrs[j].ptrShelf);
            ++idx;
        }
        for(int j = 0; j < listCount; ++j){
            if(i == prodList[j].listIdx){
                if(prodList[j].shelfIdx == -1) {
                    printf(" %d. %s -> N/A\n", idx, prodList[j].prodName);
                    idx++;
                }
            }
        }
        idx = 0;
        free(ptrs);
    }



    freeShelfs(shelfs,shelfProdCount);
    freeShopList(prodList,listCount);
    return 0;
}

TSHELF * readInputShelf(int * prodCount){
    *prodCount = 0;
    int shelfNum = -1;
    int prodSize = 10;
    TSHELF * data = (TSHELF*)malloc(prodSize*sizeof(*data));
    const int inputSize = 100000;
    char inputStr[inputSize];
    char * flag;

    while(true){
        flag = fgets(inputStr,inputSize,stdin);
        if(inputStr[0] == '\n'){
            if(shelfNum == 0){
                return data;
            }
            break;
        }
         else if(!flag || ((inputStr[0] == '\n') && *prodCount == 0) || (shelfNum == -1 && inputStr[0] != '#')
           || (inputStr[0] == '#' && (atoi(inputStr + 1) != (shelfNum + 1))) || (strlen(inputStr) - 1 == 1
           ) || (inputStr[0] == '#' && !(inputStr[1] >= '0' && inputStr[1] <= '9' ))){
            freeShelfs(data,*prodCount);
            return NULL;
        }
        if(inputStr[0] == '#'){
            shelfNum++;
        } else {
            inputStr[strlen(inputStr) - 1] = '\0';
            data[*prodCount].number = shelfNum;
            data[*prodCount].prodName = (char*)malloc((strlen(inputStr)+1) * sizeof(*data[*prodCount].prodName));
            strcpy(data[*prodCount].prodName,inputStr);
            (*prodCount)++;
            if(*prodCount == prodSize) {
                prodSize *= 2;
                data = (TSHELF*) realloc(data,prodSize * sizeof(*data));
            }
        }
    }
    return data;
}

TSHOPLIST * readShopList(int * listCount, int * countCounts){
    *listCount = 0;
    *countCounts = 0;
    int listIdx = 0;
    int listSize = 10;
    TSHOPLIST * data = (TSHOPLIST*) malloc(listSize * sizeof(*data));
    const int inputSize = 100000;
    char inputStr[inputSize];
    char * flag;
    while(true){
        flag = fgets(inputStr,inputSize,stdin);
        if(*listCount == 0){
            if(!flag || inputStr[0] == '\n'){
                freeShopList(data,*listCount);
                return NULL;
            }
        }
        if(feof(stdin)){
            if(inputStr[strlen(inputStr) - 1] != '\n'){
                data[*listCount].listIdx = listIdx;
                data[*listCount].prodName = (char*) malloc(strlen(inputStr) + 1);
                strcpy(data[*listCount].prodName,inputStr);
                (*listCount)++;
            }
            (*countCounts)++;
            break;
        }

        if(inputStr[0] == '\n'){
            listIdx++;
            (*countCounts)++;
            continue;
        }

        inputStr[strlen(inputStr) - 1] = '\0';
        data[*listCount].listIdx = listIdx;
        data[*listCount].prodName = (char*) malloc(strlen(inputStr) + 1);
        strcpy(data[*listCount].prodName,inputStr);
        (*listCount)++;
        inputStr[strlen(inputStr)] = '\n';
        if((*listCount) == listSize){
            listSize *= 2;
            data = (TSHOPLIST*) realloc(data,listSize * sizeof(*data));
        }
    }

    return data;
}

void findOptList(TSHELF * shelfs,int shelfCount, TSHOPLIST * list, int listCount){
    char * listStr;
    char * shelfStr;
    int subIdx;
    bool isFound;
    bool isSub;
    for(int i = 0; i < listCount; ++i){
        isFound = false;
        isSub = false;
        listStr = toLowerStr(list[i].prodName);
        for(int j = 0; j < shelfCount; ++j){
            shelfStr = toLowerStr(shelfs[j].prodName);
            if(!strcmp(listStr,shelfStr)){
                list[i].shelfIdx = j;
                isFound = true;
                free(shelfStr);
                break;
            } else if(strstr(shelfStr,listStr) && !isSub){
                subIdx = j;
                isSub = true;
            }
            free(shelfStr);
        }
        free(listStr);
        if(!isFound) {
            list[i].shelfIdx = isSub ? subIdx : -1;
        }
    }
}

void freeShelfs(TSHELF * shelfs, int count){
    for(int i = 0; i < count; ++i){
        free(shelfs[i].prodName);
    }
    free(shelfs);
}
void freeShopList(TSHOPLIST * list,int count){
    for(int i = 0; i < count; ++i){
        free(list[i].prodName);
    }
    free(list);
}

char * toLowerStr(char * str){
    char * result = (char*)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(result,str);
    for(int i = 0; result[i]; i++){
        result[i] = tolower(result[i]);
    }
    return result;
}

int cmpList(const void * a, const void * b){
    TPOINTERS * first = (TPOINTERS *)a;
    TPOINTERS * second = (TPOINTERS *)b;
    return first->shelfIdx - second->shelfIdx;
}
