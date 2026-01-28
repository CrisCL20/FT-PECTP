/* Some utility functions (not part of the algorithm) */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "global.h"
#include "rand.h"

/* Function to return the maximum of two variables */
double maximum(double a, double b)
{
    if (a > b)
    {
        return (a);
    }
    return (b);
}

/* Function to return the minimum of two variables */
double minimum(double a, double b)
{
    if (a < b)
    {
        return (a);
    }
    return (b);
}

char *strdup(const char *s)
{
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p)
    {
        memcpy(p, s, size);
    }
    return p;
}

char **str_split(char *a_str, const char a_delim)
{
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *tmp2 = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result)
    {
        size_t idx = 0;
        char *token = strtok(tmp2, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

int cmpactivity(t_activity a1, t_activity a2)
{
    if (strcmp(a1.id, a2.id) == 0)
        return 0;
    return 1;
}

int cpyactivity(t_activity a1, t_activity a2)
{
    strcpy(a1.id, a2.id);
    return 0;
}

int get_act_idx(problem_instance *pi, t_activity a)
{
    size_t i;
    for (i = 0; i < pi->nm_Activity; i++)
        if (strcmp(pi->A[i].id, a.id) == 0)
            return i;

    return -1;
}

size_t get_course_activity(problem_instance *pi, t_activity act)
{
    int c, a;
    for (c = 0; c < pi->nm_Courses; c++)
    {
        for (a = 0; a < pi->Ac[c].nm_activities; a++)
        {
            if (strcmp(pi->Ac[c].activities[a].id, act.id) == 0)
            {
                return pi->C[c].id;
            }
        }
    }

    return 0;
}

int course_in_student_preference(problem_instance *pi, int s_idx, size_t cid)
{
    int c;
    for (c = 0; c < pi->Cs[s_idx].nm_courses; c++)
        if (cid == pi->Cs[s_idx].courses[c].id)
            return c;
    return -1;
}

int get_timeslot_idx(problem_instance *pi, t_timeslot timeslot)
{
    int t;
    for (t = 0; t < pi->nm_TimeSlots; t++)
        if (strcmp(pi->T[t].ts, timeslot.ts) == 0)
            return t;
    return -1;
}

int timeslot_in_student_preference(problem_instance *pi, int s_idx, t_timeslot timeslot)
{
    int t;
    for (t = 0; t < pi->Ts[s_idx].nm_timeslots; t++)
        if (strcmp(pi->Ts[s_idx].timeslots[t].ts, timeslot.ts) == 0)
            return 1;
    return 0;
}

int cmp(const void *a, const void *b)
{
    return ((*(timeslot_counter *)b).counter - (*(timeslot_counter *)a).counter);
}

timeslot_counter *get_most_conflicted_free_timeslot(problem_instance *pi, individual *ind)
{
    int r, t, s, c, a;

    timeslot_counter *ts_counter = (timeslot_counter *)calloc(pi->nm_TimeSlots, sizeof(timeslot_counter));

    for (s = 0; s < pi->nm_Students; s++)
    {
        for (c = 0; c < pi->Cs[s].nm_courses; c++)
        {
            if (ind->student_courses[s][c])
            {
                int course_idx = pi->Cs[s].courses[c].id - 1;
                for (a = 0; a < pi->Ac[course_idx].nm_activities; a++)
                {
                    int found_act = 0;
                    for (r = 0; r < pi->nm_Rooms; r++)
                    {
                        for (t = 0; t < pi->nm_TimeSlots; t++)
                        {
                            if (strcmp(ind->gene[r][t].id, pi->Ac[course_idx].activities[a].id) == 0 && timeslot_in_student_preference(pi, s, pi->T[t]))
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

    return ts_counter;
}