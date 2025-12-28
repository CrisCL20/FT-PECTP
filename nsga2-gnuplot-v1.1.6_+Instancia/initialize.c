/* Data initializtion routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

#define PENALIZATION 1E6

typedef struct
{
    unsigned mid;
    unsigned degree;
} course_prios;

/* Function to initialize a population randomly */
void initialize_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        initialize_ind(&(pop->ind[i]), pi);
    }
    return;
}

int find_course(const problem_instance *pi, const t_activity act)
{
    size_t i, j;
    for (i = 0; i < pi->nm_Courses; ++i)
    {
        for (j = 0; j < pi->Ac[i].nm_activities; ++j)
        {
            if (strcmp(pi->Ac[i].activities[j].id, act.id) == 0)
                return pi->C[i].id - 1;
        }
    }

    return 0;
}

void set_modules_matrix(individual *ind, unsigned **mat, problem_instance *pi)
{
    int i, j, jj, r, t;

    for (i = 0; i < pi->nm_Courses; i++)
    {
        /*finding timeslot index for all activities in i*/
        int *acts_ts_idx = (int *)malloc(pi->Ac[i].nm_activities * sizeof(int));
        for (j = 0; j < pi->Ac[i].nm_activities; ++j)
            acts_ts_idx[j] = -1;
        int all_found = 0;
        for (r = 0; r < pi->nm_Rooms; r++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
            {
                for (j = 0; j < pi->Ac[i].nm_activities; ++j)
                    if (strcmp(ind->gene[r][t].id, pi->Ac[i].activities[j].id) == 0)
                        acts_ts_idx[j] = t;
                /*break when all activities have been found*/
                for (j = 0; j < pi->Ac[i].nm_activities; ++j)
                    all_found *= (int)(acts_ts_idx[j] != -1);

                if (all_found)
                    break;
            }
        }

        /*if any of the activities in course i clash, then it will be penalized*/
        for (j = 0; j < pi->Ac[i].nm_activities; ++j)
        {

            for (jj = j + 1; jj < pi->Ac[i].nm_activities; ++jj)
            {

                if (acts_ts_idx[j] == acts_ts_idx[jj])
                {
                    mat[i][i] = PENALIZATION;
                    break;
                }
            }
        }

        int *clashes_with_aj__ = (int *)calloc(pi->Ac[i].nm_activities, sizeof(int));
        t_activity **clashing_activities_course_i = (t_activity **)malloc(pi->Ac[i].nm_activities * sizeof(t_activity *));
        for (j = 0; j < pi->Ac[i].nm_activities; ++j)
            clashing_activities_course_i[j] = (t_activity *)malloc(pi->nm_Rooms - 1 * sizeof(t_activity));

        for (r = 0; r < pi->nm_Rooms; r++)
            for (j = 0; j < pi->Ac[i].nm_activities; ++j)
                if (
                    cmpactivity(ind->gene[r][acts_ts_idx[j]], EmptyActivity) == 0 &&
                    cmpactivity(ind->gene[r][acts_ts_idx[j]], pi->Ac[i].activities[j]) == 0)

                    clashing_activities_course_i[j][clashes_with_aj__[j]++] = ind->gene[r][acts_ts_idx[j]];

        for (j = 0; j < pi->Ac[i].nm_activities; ++j)
        {
            for (jj = 0; jj < clashes_with_aj__[j]; ++jj)
            {
                size_t other_course_idx = find_course(pi, clashing_activities_course_i[j][jj]);

                mat[i][other_course_idx] = 1;
            }
        }

        free(clashes_with_aj__);
        for (j = 0; j < pi->Ac[i].nm_activities; j++)
            free(clashing_activities_course_i[j]);
        free(clashing_activities_course_i);
        free(acts_ts_idx);
    }
}

int comp(const void *a, const void *b)
{
    return ((course_prios *)a)->degree - ((course_prios *)b)->degree;
}

void assign_students(individual *ind, problem_instance *pi)
{
    int i, j;
    unsigned **mod_mat = (unsigned **)malloc(pi->nm_Courses * sizeof(unsigned *));
    for (i = 0; i < pi->nm_Courses; i++)
        mod_mat[i] = (unsigned *)calloc(pi->nm_Courses, sizeof(unsigned));

    set_modules_matrix(ind, mod_mat, pi);

    int s, midx;
    for (s = 0; s < pi->nm_Students; s++)
    {
        /*get total clashes per module*/
        course_prios *priorities = (course_prios *)malloc(pi->Cs[s].nm_courses * sizeof(course_prios));
        for (midx = 0; midx < pi->Cs[s].nm_courses; midx++)
        {
            unsigned *clashes_arr = mod_mat[pi->Cs[s].courses[midx].id - 1];
            priorities[midx].mid = pi->Cs[s].courses[midx].id;
            priorities[midx].degree = 0;
            for (i = 0; i < pi->nm_Courses; i++)
                priorities[midx].degree += clashes_arr[i];
            free(clashes_arr);
        }

        /**************/
        /*do the greedy lol*/
        /**************/

        // sort priorities by degree
        qsort(priorities, pi->Cs[s].nm_courses, sizeof(course_prios), comp);

        int *assigned = (int *)calloc(pi->Cs[s].nm_courses, sizeof(int));
        int count_assigned = 0;

        for (midx = 0; midx < pi->Cs[s].nm_courses; midx++)
        {
            /*select module with least degree*/
            unsigned m_jidx = priorities[midx].mid - 1;

            int conflict = 0;
            for (j = 0; j < count_assigned; j++)
            {
                if (mod_mat[m_jidx][assigned[j]])
                {
                    conflict = 1;
                    break;
                }
            }

            /*assign module if it has no conflict*/
            if (!conflict && priorities[midx].degree <= pi->nm_Courses)
            {
                assigned[count_assigned++] = m_jidx;
                ind->student_courses[s][midx] = pi->Cs[s].courses[midx].id;
            }
        }

        free(assigned);
        free(priorities);
    }

    for (i = 0; i < pi->nm_Courses; i++)
        free(mod_mat[i]);
    free(mod_mat);
}

/* Function to initialize an individual randomly */
void initialize_ind(individual *ind, problem_instance *pi)
{
    int i, j;

    for (i = 0; i < pi->nm_Rooms; i++)
        for (j = 0; j < pi->nm_TimeSlots; j++)
            ind->gene[i][j] = EmptyActivity;

    for (i = 0; i < pi->nm_Activity; i++)
    {
        /*asignar la clase i a un salón y timeslot aleatorio*/

        while (1)
        {
            /*asegurarnos que el salón elegido sea adecuado para la clase*/
            const int room_choice_idx = rnd(0, pi->Ra[i].nm_rooms - 1);
            const unsigned rid = pi->Ra[i].rooms[room_choice_idx].id;

            int ts_choice_idx = rnd(0, pi->nm_TimeSlots - 1);

            if (cmpactivity(ind->gene[rid - 1][ts_choice_idx], EmptyActivity) == 0)
            {
                ind->gene[rid - 1][ts_choice_idx] = pi->A[i];
                break;
            }
        }
    }

    assign_students(ind, pi);

    return;
}
