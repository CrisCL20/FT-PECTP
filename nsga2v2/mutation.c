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

size_t roulette_timeslot(timeslot_counter *ts_counter, int size)
{
    double total = 0, cumprob = 0;
    int i;
    for (i = 0; i < size; i++)
        total += ts_counter[i].counter;

    if (total == 0)
        total = pow(2, -52);

    double probs[size];
    for (i = 0; i < size; i++)
        probs[i] = ts_counter[i].counter / total;

    double r = randomperc();

    for (i = 0; i < size; i++)
    {
        cumprob += probs[i];
        if (cumprob > r)
            return ts_counter[i].timeslot_idx;
    }

    return ts_counter[0].timeslot_idx;
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    // get array with most conflicted timeslots sorted from worst to best

    timeslot_counter *ts_counter = (timeslot_counter *)calloc(pi->nm_TimeSlots, sizeof(timeslot_counter));
    get_most_conflicted_free_timeslot(pi, ind, ts_counter);

    int n_tslots_to_consider = 5, i;

    timeslot_counter worst_tslots[n_tslots_to_consider];
    timeslot_counter best_tslots[n_tslots_to_consider];

    for (i = 0; i < n_tslots_to_consider; i++)
    {
        // copy top n_tslots_to_consider worst timeslots
        worst_tslots[i] = ts_counter[i];
        // copy top n_tslots_to_consider best timeslots (since ts_counter is sorted, we have to select from the last one)
        best_tslots[i] = ts_counter[pi->nm_TimeSlots - 2 * n_tslots_to_consider - i];
    }

    int t1 = roulette_timeslot(worst_tslots, n_tslots_to_consider);
    int t2 = roulette_timeslot(best_tslots, n_tslots_to_consider);

    for (int r = 0; r < pi->nm_Rooms; r++)
    {
        t_activity tmp = ind->gene[r][t1];
        ind->gene[r][t1] = ind->gene[r][t2];
        ind->gene[r][t2] = tmp;
    }

    free(ts_counter);

    return;
}