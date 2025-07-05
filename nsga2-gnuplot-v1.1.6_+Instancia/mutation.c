/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/* Function to perform mutation in a population */
void mutation_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        mutation_ind(&(pop->ind[i]), pi);
    }
    return;
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    int i, r, t;

    int tslot_idx, room_idx;

    int *all_ts = malloc(pi->T * sizeof(int));
    int *all_r = malloc(pi->R * sizeof(int));
    for (i = 0; i < pi->T; i++)
        all_ts[i] = i;

    for (i = 0; i < pi->R; i++)
        all_r[i] = i;

    shuffle(all_ts, pi->T);
    shuffle(all_r, pi->R);

    // Asignar sin solapamiento
    tslot_idx = all_ts[0];
    room_idx = all_r[0];

    free(all_ts);
    free(all_r);

    int next_slot = tslot_idx + 1;
    while (1)
    {
        if (ind->gene[room_idx][next_slot % pi->T] == 0)
        {
            ind->gene[room_idx][next_slot % pi->T] = ind->gene[room_idx][tslot_idx];
            ind->gene[room_idx][tslot_idx] = 0;
            break;
        }
        else
            next_slot++;
    }

    assign_students(ind, pi);

    return;
}