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

    unsigned c = rnd(1, pi->C);
    for (r = 0; r < pi->R; r++)
    {
        for (t = 0; t < pi->T; t++)
        {
            if (ind->gene[r][t] == c)
            {
                tslot_idx = t;
                room_idx = r;
                break;
            }
        }
    }

    /*select a random student and shift class to any preferred tslot*/
    unsigned s = rnd(0, pi->S - 1);
    char ts[10];
    for (t = 0; t < pi->tslot_prefs[s].nprefs; t++)
    {
        strcpy(ts, pi->tslot_prefs[s].tslots[t].ts);
        char **tokens = str_split(ts, '_');
        unsigned next_slot = tslot_idx + 1; /*idx of the student preference*/

        if (tokens)
        {
            unsigned d = atol(tokens[0]);
            unsigned b1 = atol(tokens[1]);

            next_slot = calculate_ts_idx(d, b1, pi->T);
        }

        free(tokens);

        if (ind->gene[room_idx][next_slot % pi->T] == 0)
        {
            ind->gene[room_idx][next_slot % pi->T] = ind->gene[room_idx][tslot_idx];
            ind->gene[room_idx][tslot_idx] = 0;
            break;
        }
    }

    assign_students(ind, pi);

    return;
}