#include "stdio.h"
#include "float.h"
#include "math.h"
#include "stdbool.h"

bool isDigistsEqual(double a, double b);
double findMiddle(double a,double b, double c);

int main()
{
    double aX, aY;
    double bX, bY;
    double cX, cY;
    int inputCorrectCount;

    printf("Bod A:\n");
    inputCorrectCount = scanf(" %lf %lf", &aX, &aY);
    if(inputCorrectCount != 2){
        printf("Nespravny vstup.\n"); 
        return 1;
    }

    printf("Bod B:\n");
    inputCorrectCount = scanf(" %lf %lf", &bX, &bY);
    if(inputCorrectCount != 2){
        printf("Nespravny vstup.\n"); 
        return 1;
    }

    printf("Bod C:\n");
    inputCorrectCount = scanf(" %lf %lf", &cX, &cY);
    if(inputCorrectCount != 2){
        printf("Nespravny vstup.\n"); 
        return 1;
    }

    if((isDigistsEqual(aX,bX) && isDigistsEqual(aY,bY)) || (isDigistsEqual(aX,cX) && isDigistsEqual(aY,cY)) 
                                                        || (isDigistsEqual(bX,cX) && isDigistsEqual(bY,cY))){
        printf("Nektere body splyvaji.\n");
        return 1;
    }
    
    double rightPart = (cX - aX)*(bY - aY);
    double leftPart = (cY - aY)*(bX - aX);
    if(isDigistsEqual(rightPart,leftPart)){
        printf("Body lezi na jedne primce.\n");
    } else {
        printf("Body nelezi na jedne primce.\n");
        return 1;
    }
    

    double midX = findMiddle(aX,bX,cX);
    double midY = findMiddle(aY,bY,cY);

    if(isDigistsEqual(aX,midX) && isDigistsEqual(aY,midY)){
        printf("Prostredni je bod A.\n");
    } else if(isDigistsEqual(bX,midX) && isDigistsEqual(bY,midY)){
        printf("Prostredni je bod B.\n");
    } else if(isDigistsEqual(cX,midX) && isDigistsEqual(cY,midY)){
        printf("Prostredni je bod C.\n");
    }

    return 0;
}

bool isDigistsEqual(double a, double b){
    double a1 = fabs(a);
    double b1 = fabs(b);
    double max = a1 > b1 ? a1 : b1;
    return (fabs(a-b) <= (DBL_EPSILON * max * 1000)); 
}

double findMiddle(double a,double b, double c){
    double max = a > b ? a : b;
    max = max > c ? max : c;
    double min = a < b ? a : b;
    min = min < c ? min : c;
    double mid = a + b + c - max - min;
    return mid;
}