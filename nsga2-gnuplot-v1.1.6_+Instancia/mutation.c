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

size_t get_act_idx(problem_instance *pi, t_activity a)
{
    size_t i;
    for (i = 0; i < pi->nm_Activity; i++)
    {
        if (strcmp(pi->A[i].id, a.id) == 0)
            return i;
    }

    return 0;
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    int r, t, tt, a, c, rr;

    size_t s_idx = rnd(0, pi->nm_Students - 1);

    // Get T - T[s]
    int *available_timeslots = (int *)calloc(pi->nm_TimeSlots - pi->Ts[s_idx].nm_timeslots, sizeof(int));
    size_t av_ts_idx = 0;
    int *real_freetime_tslot_idx = (int *)calloc(pi->Ts[s_idx].nm_timeslots, sizeof(int));
    size_t ft_ts_idx = 0;

    for (t = 0; t < pi->nm_TimeSlots; t++)
    {
        int found = 0;
        for (tt = 0; tt < pi->Ts[s_idx].nm_timeslots; ++tt)
        {
            if (strcmp(pi->T[t].ts, pi->Ts[s_idx].timeslots[tt].ts) == 0)
            {
                found = 1;
                real_freetime_tslot_idx[ft_ts_idx++] = t;
                break;
            }
        }

        if (!found)
            available_timeslots[av_ts_idx++] = t;
    }

    for (c = 0; c < pi->Cs[s_idx].nm_courses; c++)
    {
        if (ind->student_courses[s_idx][c])
        {
            size_t course_idx = pi->Cs[s_idx].courses[c].id - 1;
            for (a = 0; a < pi->Ac[course_idx].nm_activities; a++)
            {
                for (r = 0; r < pi->nm_Rooms; r++)
                {
                    int found_act = 0;
                    for (t = 0; t < pi->Ts[s_idx].nm_timeslots; t++)
                    {
                        if (strcmp(ind->gene[r][real_freetime_tslot_idx[t]].id, pi->Ac[course_idx].activities[a].id) == 0)
                        {
                            found_act = 1;
                            size_t act_idx = get_act_idx(pi, pi->Ac[course_idx].activities[a]);
                            int rooms_for_class[pi->Ra[act_idx].nm_rooms];

                            for (rr = 0; rr < pi->Ra[act_idx].nm_rooms; rr++)
                                rooms_for_class[rr] = pi->Ra[act_idx].rooms[rr].id - 1;

                            shuffle(rooms_for_class, pi->Ra[act_idx].nm_rooms);

                            // move the activity to first available timeslot in any room that it is suitable for
                            for (tt = 0; tt < av_ts_idx; tt++)
                            {
                                int swapped = 0;
                                for (rr = 0; rr < pi->Ra[act_idx].nm_rooms; rr++)
                                {
                                    if (strcmp(ind->gene[rooms_for_class[rr]][available_timeslots[tt]].id, EmptyActivity.id) == 0)
                                    {
                                        swapped = 1;
                                        ind->gene[rooms_for_class[rr]][available_timeslots[tt]] = ind->gene[r][real_freetime_tslot_idx[t]];
                                        ind->gene[r][real_freetime_tslot_idx[t]] = EmptyActivity;
                                        break;
                                    }
                                }
                                if (swapped)
                                    break;
                            }

                            break;
                        }
                    }
                    if (found_act)
                    {
                        break;
                    }
                }
            }

            break;
        }
    }

    free(available_timeslots);
    free(real_freetime_tslot_idx);

    assign_students(ind, pi);

    return;
}