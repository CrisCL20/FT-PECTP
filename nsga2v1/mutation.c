/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

typedef struct
{
    t_activity picked_act;
    size_t room_id;
} t_activity_choice;

/* Function to perform mutation in a population */
void mutation_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        double p_mut = randomperc();
        if (p_mut < pmut_bin)
            mutation_ind(&(pop->ind[i]), pi);
    }
    return;
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    // get array with most conflicted timeslots sorted from worst to best
    timeslot_counter *worst = get_most_conflicted_free_timeslot(pi, ind);

    int t1 = worst[0].timeslot_idx;
    int t2 = worst[pi->nm_TimeSlots - 1].timeslot_idx;

    for (int r = 0; r < pi->nm_Rooms; r++)
    {
        t_activity tmp = ind->gene[r][t1];
        ind->gene[r][t1] = ind->gene[r][t2];
        ind->gene[r][t2] = tmp;
    }

    return;
}