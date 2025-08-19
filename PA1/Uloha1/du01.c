#include "stdio.h"
#include "math.h"

int main()
{
    int days, prevDays;
    double creditInterest;
    double debitInterest;
    double balance;
    int deposit;
    int inputCount;

    printf("Zadejte kreditni urok [%%]:\n");
    inputCount = scanf(" %lf", &creditInterest);
    if(inputCount != 1 || creditInterest <= 0){
        printf("Nespravny vstup.\n");
        return 1;
    }
    
    printf("Zadejte debetni urok [%%]:\n");
    inputCount = scanf(" %lf", &debitInterest);
    if(inputCount != 1 || debitInterest <= 0){
        printf("Nespravny vstup.\n");
        return 1;
    }

    printf("Zadejte transakce:\n");
    inputCount = scanf(" %d , %d", &days, &deposit);
    if(inputCount != 2 || days < 0){
        printf("Nespravny vstup.\n");
        return 1;
    }

    balance = deposit;
    while(1){
        prevDays = days;
        inputCount = scanf(" %d , %d", &days, &deposit);
        if(inputCount != 2 || days <= prevDays){
            printf("Nespravny vstup.\n");
            return 1;
        }
        for(int i = 0; i < days - prevDays; ++i){
            double value = (balance / 100) * (balance > 0 ? creditInterest : debitInterest);
            double charge = floor(fabs(value) * 100) / 100;
            balance += (balance > 0 ? charge : -charge) ;
        }
        if(deposit == 0){
            break;
        } else {
            balance += deposit;
        }

    }

    printf("Zustatek: %.02lf\n",balance);

    return 0;
}