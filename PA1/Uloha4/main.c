#include <stdio.h>
#include "stdlib.h"

#define MAX 2000

int compare(const void * first, const void * second);
int * sumArr(int * arr, int count,int* newCount);
int countSame(int * arr, int count);

int main() {
    printf("Posloupnost:\n");
    int count = 0;
    int input = 0;
    int * arr = (int*)calloc(MAX,sizeof(*arr));
    int inputCount = 0;
    while(!(inputCount = scanf(" %d", &input)) || inputCount != EOF){
          count++;
        if(count > 2000 || inputCount == 0){
            printf("Nespravny vstup.\n");
            free(arr);
            return 1;
        }
        arr[count-1] = input;
    }
    if(count == 0){
        printf("Nespravny vstup.\n");
        return 1;
    }
    int newCount;
    int* arr1 = sumArr(arr,count,&newCount);
    free(arr);
    qsort(arr1,newCount,sizeof(*arr1),compare);
    int result = countSame(arr1,newCount);
    free(arr1);
    printf("Pocet dvojic: %d\n",result);
    return 0;
}

int countSame(int * arr, int count){
    int result = 0;
    int repCount = 0;
    for(int i = 0; i < count-1;++i){
        if(arr[i] == arr[i+1]){
            repCount++;
        } else if(repCount != 0){
            if(repCount == 1){
                result += 1;
            } else {
                for(int j = 1; j <= repCount; ++j){
                    result += j;
                }
            }
            repCount = 0;
        }
    }
    return result;
}


int * sumArr(int * arr, int count,int* newCount){
    int * result = (int*) calloc(MAX*50000,sizeof(*result));
    int temp = 0;
    int k = 0;
    for(int i = 0; i < count - 1; ++i){
        temp = arr[i];
        for(int j = i + 1; j < count; ++j){
            temp += arr[j];
            result[k] = temp;
            k++;
        }
    }
    *newCount = k;
    return result;
}

int compare(const void * first, const void * second){
    return (*(int*)first - *(int*)second);
}