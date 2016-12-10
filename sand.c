#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

    if (argc != 4) {
      fprintf(stderr,"usage: %s <length> <width> <center height>\n", argv[0]);
      exit(0);
    }
  
    // init before and after arrays
    int l = atoi(argv[1]); // length
    int w = atoi(argv[2]); // width
    int h = atoi(argv[3]); // center height
    // make sure length and width are odd numbers
    if (l % 2 == 0 || w % 2 == 0){
      fprintf(stderr,"length and width must be odd numbers\n");
      exit(0);
    }
    int before[l][w];
    int after[l][w];

    // set all elements to 0
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            before[i][j] = 0;
            after[i][j] = 0;
        }
    }
    // set center to h
    before[l/2][w/2] = h;
    after[l/2][w/2] = h;
    
    // keep updating until the update function returns a 1
    int (*b)[l][w] = &before;
    int (*a)[l][w] = &after;
    clock_t start, diff;
    start = clock();
    do {
        swap(l,w,&b, &a);              // swap before and after
    } while (!update(l,w,(*b),(*a)));  // update after from before
    diff = clock() - start; 
    printf("Single thread took %lu cycles\n", diff);
}
