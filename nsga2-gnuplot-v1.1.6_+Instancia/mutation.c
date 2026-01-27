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

int cmp(const void *a, const void *b)
{
    return ((*(timeslot_counter *)a).counter - (*(timeslot_counter *)b).counter);
}

int ts_in_student_preference(timeslot_preference student_preference, char *ts)
{
    int t;
    for (t = 0; t < student_preference.nm_timeslots; t++)
        if (strcmp(ts, student_preference.timeslots[t].ts))
            return 1;

    return 0;
}

int get_most_conflicted_free_timeslot(problem_instance *pi, individual *ind)
{
    int r, t, s, c, a;

    timeslot_counter *ts_counter = (timeslot_counter *)calloc(pi->nm_TimeSlots, sizeof(timeslot_counter));

    for (s = 0; s < pi->nm_Students; s++)
    {
        for (c = 0; c < pi->Cs[s].nm_courses; c++)
        {
            if (ind->student_courses[s][c])
            {
                for (a = 0; a < pi->Ac[pi->Cs[s].courses[c].id - 1].nm_activities; a++)
                {
                    int found_act = 0;
                    for (r = 0; r < pi->nm_TimeSlots; r++)
                    {
                        for (t = 0; t < pi->nm_TimeSlots; t++)
                        {
                            if (strcmp(ind->gene[r][t].id, pi->Ac[pi->Cs[s].courses[c].id - 1].activities[a].id) == 0 && ts_in_student_preference(pi->Ts[s], pi->T[t].ts))
                            {
                                found_act = 1;
                                ts_counter[t].counter++;
                                ts_counter[t].timeslot_idx = t;
                                break;
                            }
                        }
                        if (found_act)
                            break;
                    }
                }
            }
        }
    }

    qsort(ts_counter, pi->nm_TimeSlots, sizeof(timeslot_counter), cmp);

    size_t worst_idx = ts_counter[pi->nm_TimeSlots - 1].timeslot_idx;
    free(ts_counter);

    return worst_idx;
}

t_activity_choice pick_activity(problem_instance *pi, individual *ind, int worst_ts)
{
    int r, a, count_act = 0;
    for (r = 0; r < pi->nm_Rooms; r++)
        if (strcmp(ind->gene[r][worst_ts].id, EmptyActivity.id) != 0)
            count_act++;

    int activities_in_ts_index[count_act];
    for (a = 0; a < count_act; a++)
        activities_in_ts_index[a] = a;

    t_activity_choice activities_in_ts[count_act];
    int activities_idx = 0;
    for (r = 0; r < pi->nm_Rooms; r++)
        if (strcmp(ind->gene[r][worst_ts].id, EmptyActivity.id) != 0)
            activities_in_ts[activities_idx++] = (t_activity_choice){.picked_act = ind->gene[r][worst_ts], .room_id = r};

    shuffle(activities_in_ts_index, count_act);

    return activities_in_ts[activities_in_ts_index[0]];
}

int is_feasible(problem_instance *pi, individual *ind, t_activity a, int ts)
{
    size_t act_idx = get_act_idx(pi, a);
    int r;

    for (r = 0; r < pi->Ra[act_idx].nm_rooms; r++)
        if (strcmp(ind->gene[pi->Ra[act_idx].rooms[r].id - 1][ts].id, EmptyActivity.id) == 0)
            return 1;

    return 0;
}

int count_free_time_violations(problem_instance *pi, individual *ind, t_activity act, int ts)
{
    int s, c_idx, count = 0;
    int course_idx = get_course_activity(pi, act);
    for (s = 0; s < pi->nm_Students; s++)
    {
        c_idx = course_in_student_preference(pi, s, course_idx);
        if (ts_in_student_preference(pi->Ts[s], pi->T[ts].ts) && c_idx != -1 && ind->student_courses[s][c_idx])
            count++;
    }

    return count;
}

void swap_activity(problem_instance *pi, individual *ind, t_activity_choice act_choice, int previous_timeslot, int new_timeslot)
{
    int r;
    int act_idx = get_act_idx(pi, act_choice.picked_act);

    for (r = 0; r < pi->Ra[act_idx].nm_rooms; r++)
    {
        if (strcmp(ind->gene[pi->Ra[act_idx].rooms[r].id - 1][new_timeslot].id, EmptyActivity.id) == 0)
        {
            ind->gene[pi->Ra[act_idx].rooms[r].id - 1][new_timeslot] = act_choice.picked_act;
            ind->gene[act_choice.room_id][previous_timeslot] = EmptyActivity;
            break;
        }
    }

    return;
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    int t;
    int worst_tslot = get_most_conflicted_free_timeslot(pi, ind);
    t_activity_choice act_to_swap = pick_activity(pi, ind, worst_tslot);

    int best_target_ts = -1;
    int min_new_conflicts = 1e9;

    for (t = 0; t < pi->nm_TimeSlots; t++)
    {
        if (is_feasible(pi, ind, act_to_swap.picked_act, t))
        {
            int score = count_free_time_violations(pi, ind, act_to_swap.picked_act, t);
            if (score < min_new_conflicts)
            {
                min_new_conflicts = score;
                best_target_ts = t;
            }
        }
    }

    // swap activity from worst_tslot to new best_target_ts
    if (best_target_ts != -1)
        swap_activity(pi, ind, act_to_swap, worst_tslot, best_target_ts);

    assign_students(ind, pi);

    return;
}