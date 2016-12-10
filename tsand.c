#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
// the rules of the sandpile for a region of the sandpile
// returns 0 if the pile has changed from before to after and 1
// if the pile has not changed
// I don't use mutexes because the barrier ensures that no thread
// will be writing to before while another is reading from before
// (because this function does not modify before, so the only time
// before gets written to is when it is considered "after" by some
// thread)
// and each thread only writes to its own region of after
int update_region(int l, int w, int before[l][w], int after[l][w], int xmin, int xmax, int ymin, int ymax){
    int nochange = 1;
    for (int i = ymin; i < ymax; i++){
        for(int j = xmin; j < xmax; j++){
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

#define NO_THREADS 4 //number of threads

int barrier = 0;
int tally = 0; // tally access will also be limited by barrier mutex
pthread_mutex_t barrier_mutex;
pthread_cond_t barrier_cond;

// uses global variables to make threads wait until all are finished
// also takes a "vote" and has each thread vote on if they think the
// simulation is finished, if they think it is, the simulation ends
int barrier_wait(int vote){
    int local_tally = 0;
    pthread_mutex_lock(&barrier_mutex);
    // so when tally = NO_THREADS, the simulation is over
    tally += vote;
    if (barrier >= (NO_THREADS -1)){
        barrier = 0;
        if (tally < NO_THREADS) { 
            // reset tally if threads disagree
            tally = 0;
        }
        for (int i = 1; i < NO_THREADS; i++){
            pthread_cond_signal(&barrier_cond);
        }
    }
    else {
        barrier += 1;
        pthread_cond_wait(&barrier_cond, &barrier_mutex);
    }
    local_tally = tally;
    pthread_mutex_unlock(&barrier_mutex);
    return local_tally;
}

typedef struct _region_t {
    int l;          // size of the whole pile
    int w;
    void *b;        // pointers to before and after arrays 
    void *a; 
    int xmin;       // boundaries of this threads region
    int xmax;       // of the pile
    int ymin;
    int ymax;
    int tno;        // thread number
} region_t;

// thread for handling a region of the sandpile
void *sand_region(void *ri){
    region_t *r = (region_t *)ri;
    // pointers to before and after arrays
    int (*b)[r->l][r->w] = (int (*)[r->l][r->w])r->b; 
    int (*a)[r->l][r->w] = (int (*)[r->l][r->w])r->a; 
    do {
        if (r->tno == 1) {
            print2DArray(r->l, r->w, (*a)); // print out after
            for (int i = 0; i < r->w; i++){ // make a horizontal bar
                printf("--");
            }
            printf("\n");
        }
        swap(r->l,r->w,&b, &a);    // swap before and after
    // so, update_region will return with if the region has changed
    // or not, then that value gets passed as this region's "vote"
    // into barrier_wait to see if the whole sandpile has changed or
    // not, then that function returns how many threads' regions 
    // have not changed, and if thats greater than or equal to 
    // the number of threads, then the sandpile has quiesed
    } while(barrier_wait(
    update_region(r->l,r->w,(*b),(*a),
                  r->xmin,r->xmax,r->ymin,r->ymax)) 
            < NO_THREADS);
    pthread_exit(NULL);
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
    
    // make threads 
    int (*b)[l][w] = &before;
    int (*a)[l][w] = &after;
    pthread_t threads[NO_THREADS];
    region_t *regions[NO_THREADS];
    for (int i = 0; i < NO_THREADS; i++){
        regions[i] = (region_t *)malloc(sizeof(region_t));
        regions[i]->l = l;
        regions[i]->w = w;
        regions[i]->b = (void *)b;
        regions[i]->a = (void *)a;
        // each region will take up the entire width of the sandpile
        // but only 1/NO_THREADS of the length
        regions[i]->xmin = 0;
        regions[i]->xmax = w;
        // the ymax of one thread will be the ymin of the next
        // but thats okay because update_region updates the "ymin"th
        // up to the "ymax-1"th row
        regions[i]->ymin = (i*l)/NO_THREADS;
        regions[i]->ymax = ((i+1)*l)/NO_THREADS;
        regions[i]->tno = i+1;
    }
    pthread_cond_init (&barrier_cond, NULL);
    pthread_mutex_init(&barrier_mutex, NULL);
    for (int i = 0; i < NO_THREADS; i++){
        pthread_create(&threads[i], NULL, sand_region, (void *)regions[i]);
    }
    for (int i = 0; i < NO_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_exit(NULL);
}
