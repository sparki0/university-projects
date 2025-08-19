#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stdbool.h"

typedef struct TItem
{
    struct TItem * m_Next;
    char * m_Name;
    char m_Secret[24];
} TITEM;

int cmpName ( const TITEM * a, const TITEM * b )
{
    return strcmp(a -> m_Name, b -> m_Name );
}

int cmpNameInsensitive ( const TITEM * a, const TITEM * b )
{
    return strcasecmp( a -> m_Name, b -> m_Name );
}

int cmpNameLen (const TITEM * a, const TITEM * b )
{
    size_t la = strlen ( a -> m_Name ), lb = strlen ( b -> m_Name );
    return ( lb < la ) - ( la < lb );
}
#endif /* __PROGTEST__ */

TITEM * swapTITEM(TITEM * a, TITEM * b){
    TITEM * temp = b->m_Next;
    b->m_Next = a;
    a->m_Next = temp;
    return b;
}

TITEM * newItem( const char * name, TITEM * next )
{
    TITEM * resultPtr = (TITEM*) malloc(sizeof(*resultPtr));
    resultPtr->m_Name = (char*)malloc((strlen(name) + 1) * sizeof(*name));
    strcpy(resultPtr->m_Name, name);
    resultPtr->m_Secret[0] = '\0';
    resultPtr->m_Next = next;
    return resultPtr;
}
TITEM * sortListCmp( TITEM * l, int ascending, int (* cmpFn) ( const TITEM *, const TITEM *) )
{
    TITEM * firstPtr;
    TITEM * secondPtr;

    TITEM ** newBeg;

    int result1;
    int result2;
    int koeff = ascending ? 1 : -1;
    int listCount = 0;

    for(TITEM * temp = l; temp; temp = temp->m_Next, ++listCount) {};
    for(int i = 0; i <= listCount; ++i){
        newBeg = &l;
        bool isSorted = false;
        for(int j = i + 1; j < listCount; ++j, newBeg = &(*newBeg)->m_Next){
            firstPtr = *newBeg;
            secondPtr = firstPtr->m_Next;
            result1 = cmpFn(firstPtr, secondPtr) * koeff;
            result2 = cmpFn(secondPtr, firstPtr) * koeff;
            if(result1 > result2){
                *newBeg = swapTITEM(firstPtr,secondPtr);
                isSorted = true;
            }

        }

        if(!isSorted){
            break;
        }
    }

    return l;
}
void freeList( TITEM * src )
{
    for(TITEM * cur = src; cur ;){
        free(cur->m_Name);
        TITEM * temp = cur;
        cur = cur->m_Next;
        free(temp);
    }
    src = NULL;
}


#ifndef __PROGTEST__
int main ( int argc, char * argv [] )
{
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
