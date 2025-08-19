#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>

/* Airplane's maximum length name*/
#define MAX_LEN 200

/* TAirplane is structure that contains airplane coordinates (x and y)
 * and his name in char array, name contains maximum of 200 char
 */
typedef struct TAirplane{
    double x,y;
    char name[MAX_LEN];
} TAIRPLANE;
/* TPair is structure that contains index of airplane, that will have collision
 */

typedef struct TPair{
    int first;
    int second;
} TPAIR;

TAIRPLANE* readInput(int * count);
double calculateDist(const TAIRPLANE* a,const TAIRPLANE* b);
double findMinDist(const TAIRPLANE * airplanes, int airplaneCount);
TPAIR * findCollision(const TAIRPLANE* airplanes, int airplaneCount,double minDist, int * collisionCount);
bool areDigistsEqual(double a, double b);

int main() {
    int airplaneCount = 0;
    int collisionCount = 0;

    printf("Pozice letadel:\n");
    TAIRPLANE * airplanes = readInput(&airplaneCount);
    if(!airplanes){
        printf("Nespravny vstup.\n");
        return 1;
    }

    double minDist = findMinDist(airplanes,airplaneCount);

    TPAIR * collisions = findCollision(airplanes, airplaneCount,minDist,&collisionCount);
    printf("Vzdalenost nejblizsich letadel: %lf\n",minDist);
    printf("Nalezenych dvojic: %d\n",collisionCount);

    for(int i = 0; i < collisionCount; ++i){
        printf("%s - %s\n",airplanes[collisions[i].first].name,airplanes[collisions[i].second].name);
    }

    free(airplanes);
    free(collisions);
    return 0;
}

/*
 * Read two doubles and string from input and writes data in array
 * Reading is not successful:
 * -if argument count is not equal to 3
 * -if coordinates or name are not in correct form (for example invalid number)
 * -if input count is less than 2
 * @param[out] count count of data array
 * @returns  pointer to array of TAIRPLANE if inputs were all correct, else returns NULL  
*/

TAIRPLANE* readInput(int * count){
    int size = 10;
    *count = 0;
    TAIRPLANE * result = (TAIRPLANE*) malloc(size*sizeof(*result));
    int argCount;
    while((argCount = scanf(" %lf , %lf : %199s",&result[*count].x,&result[*count].y,result[*count].name)) != EOF){
        if(argCount != 3){
            free(result);
            return NULL;
        }
        ++(*count);
        if((*count) == size){
            size *= 2;
            result = (TAIRPLANE*)realloc(result,size*sizeof(*result));
        }
    }
    if(*count < 2){
        free(result);
        return NULL;
    }
    return result;
}

/* finds minimum distance between airplanes
 * @param[in] airplanes pointer to array of TAIRPLANE
 * @param[in] airplaneCount counts of array's elements
 * @returns minimum distance between two airplanes
 */

double findMinDist(const TAIRPLANE * airplanes, int airplaneCount){
    double minDist = calculateDist(&airplanes[0],&airplanes[1]);
    for(int i = 0; i < airplaneCount - 1; ++i){
        for(int j = i + 1; j < airplaneCount; ++j){
              double dist = calculateDist(&airplanes[i],&airplanes[j]);
              if(dist < minDist){
                  minDist = dist;
              }
        }
    }

    return minDist;
}

/* find collision between airplanes
 * @param[in] airplanes pointer to array of TAIRPLANE
 * @param[in] airplaneCount counts of array's elements
 * @param[in] minDist distance between airplanes, that will have collions
 * @param[out] collisionCount count of collisions
 * @returns array of TPAIR
 */

TPAIR * findCollision(const TAIRPLANE* airplanes, int airplaneCount,double minDist, int * collisionCount){
    int collisionSize = 10;
    *collisionCount = 0;
    TPAIR * result = (TPAIR*) malloc(collisionSize*sizeof(*result));
    for(int i = 0; i < airplaneCount - 1;++i) {
        for (int j = i + 1; j < airplaneCount; ++j) {
            double dist = calculateDist(&airplanes[i], &airplanes[j]);
            if (areDigistsEqual(minDist, dist)) {
                /*if distances between two airplanes is equal to minimum distance, write the index of element in array airplanes to result array*/
                result[*collisionCount].first = i;
                result[*collisionCount].second = j;
                (*collisionCount)++;
                if (collisionSize == *collisionCount) {
                    collisionSize *= 2;
                    result = (TPAIR *) realloc(result, collisionSize * sizeof(*result));
                }
            }
        }
    }
    return result;
}

/* calculates distance between airplanes
 * @param[in] a pointer to first airplane
 * @param[in] b pointer to second airplane
 * returns distance between a and b
 */

double calculateDist(const TAIRPLANE* a,const TAIRPLANE* b){
    double x = b->x - a->x;
    double y = b->y - a->y;
    return sqrt((x*x)+(y*y));
}

/* compares two doubles if they are equal
 * @param[in] a 
 * @param[in] b
 * @returns true if doubles are equal, else returns false
 */
bool areDigistsEqual(double a, double b){
    double a1 = fabs(a);
    double b1 = fabs(b);
    double max = a1 > b1 ? a1 : b1;
    return (fabs(a-b) <= (DBL_EPSILON * max * 1000000));
}