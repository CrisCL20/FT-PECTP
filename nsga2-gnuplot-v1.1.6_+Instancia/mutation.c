/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

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
    int i, r, t;

    int tslot_idx = 0, room_idx = 0;

    unsigned c = rnd(0, pi->nm_Activity - 1);
    for (r = 0; r < pi->nm_Rooms; r++)
    {
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            if (cmpactivity(ind->gene[r][t], pi->A[c]) == 0)
            {
                tslot_idx = t;
                room_idx = r;
                break;
            }
        }
    }

    /*select a random student and shift class to any preferred tslot*/
    unsigned s = rnd(0, pi->nm_Students - 1);
    char ts[10];
    for (t = 0; t < pi->Ts[s].nm_timeslots; t++)
    {
        strcpy(ts, pi->Ts[s].timeslots[t].ts);
        char **tokens = str_split(ts, '_');
        unsigned next_slot = tslot_idx + 1; /*idx of the student preference*/

        if (tokens)
        {
            unsigned d = atol(tokens[0]);
            unsigned b1 = atol(tokens[1]);

            next_slot = calculate_ts_idx(d, b1, pi->nm_TimeSlots);
        }

        free(tokens);

        if (cmpactivity(ind->gene[room_idx][next_slot % pi->nm_TimeSlots], EmptyActivity) == 0)
        {
            ind->gene[room_idx][next_slot % pi->nm_TimeSlots] = ind->gene[room_idx][tslot_idx];
            ind->gene[room_idx][tslot_idx] = EmptyActivity;
            break;
        }
    }

    assign_students(ind, pi);

    return;
}