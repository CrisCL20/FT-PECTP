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
    int i, j;

    ind->gene = (unsigned **)malloc(pi->R * sizeof(unsigned *));
    for (i = 0; i < pi->R; i++)
        ind->gene[i] = (unsigned *)malloc(pi->T * sizeof(unsigned));

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

    for (i = 0; i < pi->S; i++)
        free(ind->student_modules[i]);
    free(ind->student_modules);

    for (i = 0; i < pi->R; i++)
        free(ind->gene[i]);
    free(ind->gene);

    free(ind->obj);
    if (ncon != 0)
    {
        free(ind->constr);
    }
    return;
}
