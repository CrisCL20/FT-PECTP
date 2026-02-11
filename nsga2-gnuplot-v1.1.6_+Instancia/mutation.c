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

t_activity_choice pick_activity(problem_instance *pi, individual *ind, int worst_ts)
{
    int r, a, count_act = 0;
    for (r = 0; r < pi->nm_Rooms; r++)
        if (strcmp(ind->gene[r][worst_ts].id, EmptyActivity.id) != 0)
            count_act++;

    if (count_act == 0)
    {
        t_activity_choice nullchoice = (t_activity_choice){.picked_act = EmptyActivity, .room_id = 0};
        return nullchoice;
    }

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

int count_free_time_violations(problem_instance *pi, individual *ind, t_activity act, int ts)
{
    int s, c_idx, count = 0;
    int course_idx = get_course_activity(pi, act);
    for (s = 0; s < pi->nm_Students; s++)
    {
        c_idx = course_in_student_preference(pi, s, course_idx);
        if (timeslot_in_student_preference(pi, s, pi->T[ts]) && c_idx != -1 && ind->student_courses[s][c_idx])
            count++;
    }

    return count;
}

void swap_activity(problem_instance *pi, individual *ind, t_activity_choice act_choice, int previous_timeslot, int new_timeslot)
{
    int r;
    for (r = 0; r < pi->nm_Rooms; r++)
    {
        if (strcmp(ind->gene[r][new_timeslot].id, EmptyActivity.id) == 0)
        {
            ind->gene[r][new_timeslot] = act_choice.picked_act;
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

    timeslot_counter *bad_tslots = get_most_conflicted_free_timeslot(pi, ind);
    int worst_tslot = bad_tslots[0].timeslot_idx;
    free(bad_tslots);

    t_activity_choice act_to_swap = pick_activity(pi, ind, worst_tslot);

    int best_target_ts = -1;
    int min_new_conflicts = 1e9;

    for (t = 0; t < pi->nm_TimeSlots; t++)
    {

        int score = count_free_time_violations(pi, ind, act_to_swap.picked_act, t);
        if (score < min_new_conflicts)
        {
            min_new_conflicts = score;
            best_target_ts = t;
        }
    }

    // swap activity from worst_tslot to new best_target_ts
    if (best_target_ts != -1)
        swap_activity(pi, ind, act_to_swap, worst_tslot, best_target_ts);

    assign_students(ind, pi);

    return;
}