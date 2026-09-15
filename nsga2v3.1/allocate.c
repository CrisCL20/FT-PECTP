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
    int i;

    ind->gene = (size_t **)malloc(pi->nm_Rooms * sizeof(size_t *));
    for (i = 0; i < pi->nm_Rooms; i++)
    {
        ind->gene[i] = (size_t *)malloc(pi->nm_TimeSlots * sizeof(size_t));
        for (int j = 0; j < pi->nm_TimeSlots; j++)
            ind->gene[i][j] = EMPTY_ACT;
    }

    ind->course_sat = (t_course_sat *) calloc(pi->nm_Courses, sizeof(t_course_sat));
    
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

    for (i = 0; i < pi->nm_Rooms; i++)
        free(ind->gene[i]);
    free(ind->gene);
    free(ind->course_sat);

    free(ind->obj);

    return;
}

void deallocate_memory_instance(problem_instance *pi)
{
    int i;
    for (i = 0; i < pi->nm_Students; i++)
    {
        free(pi->Cs[i].courses);
        free(pi->Ts[i].timeslots);
    }

    free(pi->S);

    for (i = 0; i < pi->nm_Activity; i++)
        free(pi->Ra[i].rooms);

    for (i = 0; i < pi->nm_Courses; i++)
    {
        free(pi->Ac[i].activities);
        free(pi->Ac[i].activity_idx);
    }

    free(pi->A);
    free(pi->C);
    free(pi->T);
    free(pi->R);
    free(pi->colors);
}
