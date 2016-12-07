#include <stdio.h>

#define HEIGHT_MAX 16

// Prints out a 2d array
void print2DArray(int l, int w, int a[l][w]){
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            printf("%d ",a[i][j]);
        }
        printf("\n");
    }
}

int main() {
    int l = 3; // length
    int w = 3; // width
    int before[l][w];
    // set all elements to max
    for (int i = 0; i < l; i++){
        for(int j = 0; j < w; j++){
            before[i][j] = HEIGHT_MAX;
        }
    }
    

    print2DArray(l, w, before);

}
