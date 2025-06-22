/* Memory allocation and deallocation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/* Function to allocate memory to a population */
void allocate_memory_pop(population *pop, int size, problem_instance *pi)
{
    int i;
    pop->ind = (individual *)malloc(size * sizeof(individual));
    for (i = 0; i < size; i++)
    {
        allocate_memory_ind(&(pop->ind[i]), pi);
    }
    return;
}

/* Function to allocate memory to an individual */
void allocate_memory_ind(individual *ind, problem_instance *pi)
{
    int j;
    /*
    if (nreal != 0)
    {
        ind->xreal = (double *)malloc(nreal*sizeof(double));
    }
    if (nbin != 0)
    {
        ind->xbin = (double *)malloc(nbin*sizeof(double));
        ind->gene = (int **)malloc(nbin*sizeof(int *));
        for (j=0; j<nbin; j++)
        {
            ind->gene[j] = (int *)malloc(nbits[j]*sizeof(int));
        }
    }
    ind->obj = (double *)malloc(nobj*sizeof(double));
    if (ncon != 0)
    {
        ind->constr = (double *)malloc(ncon*sizeof(double));
    }
    */

    ind->gene = (room_ts_pair *)malloc(pi->C * sizeof(room_ts_pair));
    ind->student_modules = (unsigned **)malloc(pi->S * sizeof(unsigned *));
    for (j = 0; j < pi->S; j++)
        ind->student_modules[j] = (unsigned *)malloc(pi->mod_prefs[j].nmods * sizeof(unsigned));

    ind->obj = (double *)malloc(nobj * sizeof(double));
    if (ncon != 0)
    {
        ind->constr = (double *)malloc(ncon * sizeof(double));
    }

    return;
}

/* Function to deallocate memory to a population */
void deallocate_memory_pop(population *pop, int size, problem_instance *pi)
{
    int i;
    for (i = 0; i < size; i++)
    {
        deallocate_memory_ind(&(pop->ind[i]), pi);
    }
    free(pop->ind);
    return;
}

/* Function to deallocate memory to an individual */
void deallocate_memory_ind(individual *ind, problem_instance *pi)
{
    int i, j;
    if (nreal != 0)
    {
        free(ind->xreal);
    }

    free(ind->gene);

    free(ind->obj);
    if (ncon != 0)
    {
        free(ind->constr);
    }
    return;
}
