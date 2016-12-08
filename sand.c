#include <stdio.h>
#include <stdlib.h>

#define START_HEIGHT 16

// Prints out a 2d array
void print2DArray(int l, int w, int a[l][w]){
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            printf("%d ",a[i][j]);
        }
        printf("\n");
    }
}

// takes before and makes after the result after 1 application of
// the rules of the sandpile
// returns 0 if the pile has changed from before to after and 1
// if the pile has not changed
int update(int l, int w, int before[l][w], int after[l][w]){
    int nochange = 1;
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            // first set after to before            
            after[i][j] = before[i][j];
            // check if this pile needs toppling
            if (before[i][j] > 4){
                after[i][j] -= 4;
            }
            // check if adjacent tiles are toppling
            if (i > 0 && before[i-1][j] > 4) {
                after[i][j] += 1;
            }
            if (j > 0 && before[i][j-1] > 4) {
                after[i][j] += 1;
            }
            if (i < (l-1) && before[i+1][j] > 4) {
                after[i][j] += 1;
            }
            if (j < (w-1) && before[i][j+1] > 4) {
                after[i][j] += 1;
            }
            // if after is different from before, set nochange to 0
            if (after[i][j] != before[i][j]){
                nochange = 0; 
            }
        }
    }
    return nochange;
}

// swaps 2 2d int arrays
// I stole this swap function from here:
// http://stackoverflow.com/questions/33983368/c-swapping-two-2d-arrays-by-switching-pointers
void swap(int l, int w, int (**a)[l][w], int (**b)[l][w]) {
    int (*temp)[l][w] = *a;
    *a = *b;
    *b = temp;
}

int main(int argc, char **argv) { 

    if (argc != 3) {
      fprintf(stderr,"usage: %s <length> <width>\n", argv[0]);
      exit(0);
    }
  
    // init before and after arrays
    int l = atoi(argv[1]); // length
    int w = atoi(argv[2]); // width
    int before[l][w];
    int after[l][w];

    // set all elements to start height
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            before[i][j] = START_HEIGHT;
            after[i][j] = START_HEIGHT;
        }
    }
    // keep updating until the update function returns a 1
    int (*b)[l][w] = &before;
    int (*a)[l][w] = &after;
    do {
        print2DArray(l, w, (*a));      // print out after
        swap(l,w,&b, &a);              // swap before and after
        printf("Time Step!\n");        //
    } while (!update(l,w,(*b),(*a)));  // update after from before
}
