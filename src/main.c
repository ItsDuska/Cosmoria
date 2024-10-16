#include <stdio.h>
#include "engine/engineCore.h"

int main(int a, char** b)
{
    printf("Hello, from Cosmoria!\n");
    createEngine();
    runEngine();
    destroyEngine();
    return 0; 
}

