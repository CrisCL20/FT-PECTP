/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

typedef struct
{
    size_t timeslot_idx;
    size_t counter;
} timeslot_counter;

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

int cmp(const void *a, const void *b)
{
    return ((*(timeslot_counter *)a).counter - (*(timeslot_counter *)b).counter);
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    int r, t;

    /* select random activity and its timeslot student counter */
    unsigned act_idx = rnd(0, pi->nm_Activity - 1);
    int original_tslot_idx = 0, room_idx = 0;
    for (r = 0; r < pi->nm_Rooms; r++)
    {
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            if (cmpactivity(ind->gene[r][t], pi->A[act_idx]) == 0)
            {
                original_tslot_idx = t;
                room_idx = r;
                break;
            }
        }
    }

    size_t s, c, a, found_activity, original_tslot_student_counter = 0, tmp_tslot_idx;
    timeslot_counter *timeslots_counter = (timeslot_counter *)calloc(pi->nm_TimeSlots, sizeof(timeslot_counter));

    for (s = 0; s < pi->nm_Students; ++s)
    {
        for (c = 0; c < pi->Cs[s].nm_courses; ++c)
        {
            for (a = 0; a < pi->Ac[c].nm_activities; ++a)
            {
                // if the student wants to assist the course that has the activity...
                if (cmpactivity(pi->Ac[c].activities[a], pi->A[act_idx]) == 0)
                {

                    for (t = 0; t < pi->Ts[s].nm_timeslots; ++t)
                    {

                        char tmp_ts[10];
                        strcpy(tmp_ts, pi->Ts[s].timeslots[t].ts);
                        // printf("ESTUDIANTE %d TIMESLOT %s \n", pi->S[s].id, pi->Ts[s].timeslots[t].ts);
                        char **tokens = str_split(tmp_ts, '_');

                        if (tokens)
                        {
                            unsigned d = atol(tokens[0]);
                            unsigned b1 = atol(tokens[1]);

                            tmp_tslot_idx = calculate_ts_idx(d, b1, pi->nm_TimeSlots);
                        }

                        free(tokens);

                        // ... calculate free time request frequency
                        if (timeslots_counter[tmp_tslot_idx].counter == 0)
                            timeslots_counter[tmp_tslot_idx].timeslot_idx = tmp_tslot_idx;
                        timeslots_counter[tmp_tslot_idx].counter++;
                        // ... if free time request is the same as the original timeslot
                        if (tmp_tslot_idx == original_tslot_idx)
                        {
                            original_tslot_student_counter++;
                            break;
                        }
                    }

                    found_activity = 1;
                    break;
                }
            }
            if (found_activity)
            {
                // reset value for next student
                found_activity = 0;
                break;
            }
        }
    }

    /* if there is at least one free time request on the original timeslot, see if it can be moved to the timeslot with lowest counter */
    qsort(timeslots_counter, pi->nm_TimeSlots, sizeof(timeslot_counter), cmp);

    if (original_tslot_student_counter > 0)
    {
        // try until there is an available cell
        size_t lower_idx = 0;
        while (1)
        {

            size_t lowest = timeslots_counter[lower_idx].timeslot_idx;
            if (cmpactivity(ind->gene[room_idx][lowest], EmptyActivity) == 0)
            {
                cpyactivity(ind->gene[room_idx][lowest], pi->A[act_idx]);
                cpyactivity(ind->gene[room_idx][original_tslot_idx], EmptyActivity);
                break;
            }

            lower_idx++;
        }
    }

    assign_students(ind, pi);

    return;
}