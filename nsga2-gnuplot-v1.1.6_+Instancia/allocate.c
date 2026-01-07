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

    ind->gene = (t_activity **)malloc(pi->nm_Rooms * sizeof(t_activity *));
    for (i = 0; i < pi->nm_Rooms; i++)
        ind->gene[i] = (t_activity *)calloc(pi->nm_TimeSlots, sizeof(t_activity));

    ind->student_courses = (unsigned **)malloc(pi->nm_Students * sizeof(unsigned *));
    for (j = 0; j < pi->nm_Students; j++)
        ind->student_courses[j] = (unsigned *)calloc(pi->Cs[j].nm_courses, sizeof(unsigned));

    ind->obj = (double *)malloc(nobj * sizeof(double));

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
    int i;

    for (i = 0; i < pi->nm_Students; i++)
        free(ind->student_courses[i]);
    free(ind->student_courses);

    for (i = 0; i < pi->nm_Rooms; i++)
        free(ind->gene[i]);
    free(ind->gene);

    free(ind->obj);

    return;
}
