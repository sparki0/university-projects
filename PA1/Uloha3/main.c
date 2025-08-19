#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#endif /* __PROGTEST__ */

int bells ( int y1, int m1, int d1, int h1, int i1,
            int y2, int m2, int d2, int h2, int i2,
            long long int * b1, long long int * b2 );

int daysMonth(int monthNum, int year);
bool isSunday(int day,int month,int year);
bool isYearLeap(unsigned int year);
bool checkParams(int y1, int m1, int d1, int h1, int i1,
                 int y2, int m2, int d2, int h2, int i2);

#ifndef __PROGTEST__
int main ( int argc, char * argv [] )
{
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */

int bells ( int y1, int m1, int d1, int h1, int i1,
            int y2, int m2, int d2, int h2, int i2,
            long long int * b1, long long int * b2)
{
    if(!checkParams(y1,m1,d1,h1,i1,y2,m2,d2,h2,i2)){
        return 0;
    }

    *b1 = *b2 = 0;
    if(y1 == y2 && m1 == m2 && d1 == d2 && h1 == h2 && i1 % 15 != 0 && i2 % 15 != 0){
        return 1;
    }
    while(i1 % 15 != 0){
        ++i1;
    }
    while(i2 % 15 != 0){
        --i2;
    }

    if(y1 == y2 && m1 == m2 && d1 == d2){
        if(isSunday(d1,m1,y1))
            return 1;
        for(;h1 <= h2; i1 +=15 ){
            if (i1 == 60) {
                h1++;
                i1 = 0;
            }
            if(i1 == 0 && h1 == 0) {
                *b1 += 4;
                *b2 += 12;
            } else if (i1 == 0) {
                *b1 += 4;
                *b2 += h1 <= 12 ? h1 : h1 - 12;
            } else if (i1 == 15) {
                *b1 += 1;
            } else if (i1 == 30) {
                *b1 += 2;
            } else if (i1 == 45) {
                *b1 += 3;
            }
            if((h1 == h2 && i1 == i2) || (h1 == 23 && i1 == 45)){
                break;
            }
        }
        return 1;
    }
    if(!isSunday(d1,m1,y1)){
        for(;; i1 += 15){
            if (i1 == 60) {
                h1++;
                i1 = 0;
                if(h1 == 24){
                    break;
                }
            }

            if(i1 == 0 && h1 == 0) {
                *b1 += 4;
                *b2 += 12;
            } else if (i1 == 0) {
                *b1 += 4;
                *b2 += h1 <= 12 ? h1 : h1 - 12;
            } else if (i1 == 15) {
                *b1 += 1;
            } else if (i1 == 30) {
                *b1 += 2;
            } else if (i1 == 45) {
                *b1 += 3;
            }
        }
    }
    h1 = i1 = 0;
    ++d1;


    if(!(y2 == y1 && m2 == m1 && d1 == d2)) {
        int totalDay = 0;
        int sunday = 0;
        int total1 = 0;
        while (y1 <= y2) {
            int m3 = y1 == y2 ? m2 : 12;
            while (m1 <= m3) {
                int d3 = (y1 == y2 && m1 == m2) ? d2 : daysMonth(m1, y1) + 1;
                for (int k = d1; k < d3; ++k) {
                    ++total1;
                    if (!isSunday(k, m1, y1)) {
                        ++totalDay;
                    } else {
                        sunday++;
                    }
                }
                ++m1;
                d1 = 1;
            }
            m1 = 1;
            y1++;
        }


        *b1 += 10 * 24 * totalDay;
        *b2 += 78 * 2 * totalDay;
    }

    if(!isSunday(d2,m2,y2)) {
        for (;; i1 += 15) {
            if (i1 == 60) {
                h1++;
                i1 = 0;
            }

            if (i1 == 0 && h1 == 0) {
                *b1 += 4;
                *b2 += 12;
            } else if (i1 == 0) {
                *b1 += 4;
                *b2 += h1 <= 12 ? h1 : h1 - 12;
            } else if (i1 == 15) {
                *b1 += 1;
            } else if (i1 == 30) {
                *b1 += 2;
            } else if (i1 == 45) {
                *b1 += 3;
            }

            if (h1 == h2 && i1 == i2) {
                break;
            }
        }
    }



    return 1;
}



int daysMonth(int monthNum, int year){
    if(monthNum == 1 || monthNum == 3 || monthNum == 5 || monthNum == 7 || monthNum == 8 || monthNum == 10 || monthNum == 12){
        return 31;
    } else if(monthNum == 2){
        return isYearLeap(year) ? 29 : 28;
    } else {
        return 30;
    }
}
bool isSunday(int day,int month,int year){
    if(month < 3){
        day += year;
        year--;
    } else {
        day += year - 2;
    }
    int dayNumber = ((23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400)) % 7   ;
    return dayNumber == 0 ? true : false;
}
bool isYearLeap(unsigned int year){
    if((year % 4 == 0)){
        if(year % 4000 == 0){
            return false;
        }
        else if(year % 100 == 0){
            return year % 400 == 0 ? true : false;
        }
        else {
            return true;
        }
    }
    return false;
}
bool checkParams(int y1, int m1, int d1, int h1, int i1,
                 int y2, int m2, int d2, int h2, int i2){
    if((y1 > y2) || (y1 < 1600) || (y2 < 1600) || (m1 < 1) || (m1 > 12) || (m2 < 1) || (m2 > 12) || (d1 < 1) || (d2 < 1) || ((y1 == y2) && (m1 > m2))
       || (d1 > daysMonth(m1,y1)) || (d2 > daysMonth(m2,y2)) || (h1 < 0) || (h1 > 23) || (h2 < 0) || (h2 > 23)
       || (i1 < 0) || (i1 > 59) || (i2 < 0) || (i2 > 59)){
        return false;
    } else if(y2 == y1){
        if(m1 > m2){
            return false;
        } else if(m2 == m1){
            if(d1 > d2){
                return false;
            } else if(d2 == d1){
                if(h1 > h2){
                    return false;
                } else if(h2 == h1){
                    if(i1 > i2){
                        return  false;
                    }
                }
            }
        }
    }
    return true;
}

