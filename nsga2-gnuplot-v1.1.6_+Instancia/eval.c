/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop(population *pop, const problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        evaluate_ind(&(pop->ind[i]), pi);
    }
    return;
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

int get_ts_block_idx(const unsigned b1, const unsigned T)
{
    return ((2 * T / 5) - b1 - 1) / 2;
}

int calculate_ts_idx(unsigned d, unsigned b1, unsigned T)
{
    return ((T / 5) * d) - 1 - get_ts_block_idx(b1, T);
}

/*FO1: preferencias horarias*/
void countTimesRequestsMet(unsigned **student_schedule, t_activity **gene, double *obj, problem_instance *pi)
{
    int i, j, k, r;
    unsigned long counts = 0;

    for (i = 0; i < pi->nm_Students; i++)
    {
        /*count where the student goes to classes in a time period they did not request*/
        unsigned n_prefs = pi->Ts[i].nm_timeslots;
        char ts[10];
        for (j = 0; j < n_prefs; j++)
        {
            strcpy(ts, pi->Ts[i].timeslots[j].ts);
            char **tokens = str_split(ts, '_');
            unsigned ts_idx; /*idx of the student preference*/

            if (tokens)
            {
                unsigned d = atol(tokens[0]);
                unsigned b1 = atol(tokens[1]);

                ts_idx = calculate_ts_idx(d, b1, pi->nm_TimeSlots);
            }

            free(tokens);

            /*if the student does not attend to a class in that timeslot, increase counts*/
            /*find classes the student takes -> see if one of those classes is scheduled in ts_idx*/
            /*counts++ per each activity the student has on a free time request*/
            unsigned nm_activities = 0;
            for (unsigned j = 0; j < pi->Cs[j].nm_courses; ++j)
                nm_activities += pi->Ac[student_schedule[i][j] - 1].nm_activities;

            t_activity *student_activities = (t_activity *)malloc(nm_activities * sizeof(t_activity));
            unsigned k = 0;
            for (unsigned j = 0; j < pi->Cs[j].nm_courses; ++j)
            {

                memcpy(
                    student_activities + k,
                    pi->Ac[student_schedule[i][j] - 1].activities,
                    pi->Ac[student_schedule[i][j] - 1].nm_activities * sizeof(t_activity));
                k += pi->Ac[student_schedule[i][j]].nm_activities;
            }

            for (unsigned r = 0; r < pi->nm_Rooms; ++r)
            {
                for (unsigned a = 0; a < nm_activities; ++a)
                {
                    if (strcmp(gene[r][ts_idx].id, student_activities[a].id) == 0)
                    {
                        counts++;
                        break;
                    }
                }
            }
        }
    }

    obj[0] = counts;
}

/*FO2: cantidad de modulos preferidos no asignados*/
void countCourseRequestsMet(unsigned **students_schedule, double *obj, problem_instance *pi)
{
    int i, j;
    unsigned long counts = 0;

    for (i = 0; i < pi->nm_Students; i++)
    {
        unsigned n_required = pi->Cs[i].nm_courses, met = 0;
        for (j = 0; j < n_required; j++)
            if (students_schedule[i][j] == pi->Cs[i].courses[j].id)
                met++;

        counts += n_required - met;
    }

    obj[1] = counts;
}

void check_room_cap(individual *ind, problem_instance *pi)
{
    int r, t, s, m;

    for (r = 0; r < pi->R; r++)
    {
        unsigned room_cap = pi->rho[r];
        /*get count of students that attend a class in r per time slot*/
        /**/
        unsigned long n_students = 0;
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            char cid[10];
            strcpy(cid, ind->gene[r][t].id);
            for (s = 0; s < pi->nm_Students; s++)
            {
                for (m = 0; m < pi->Cs[s].nm_courses; m++)
                {
                    if (ind->student_courses[s][m] == pi->Cs[s].courses[m].id)
                    {
                        t_activity *course_activities = (t_activity *)malloc(pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities * sizeof(t_activity));
                        memcpy(
                            course_activities,
                            pi->Ac[pi->Cs[s].courses[m].id - 1].activities,
                            pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities * sizeof(t_activity));

                        for (unsigned a = 0; a < pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities; ++a)
                        {

                            if (strcmp(cid, course_activities[a].id) == 0)
                            {
                                n_students++;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (n_students > room_cap)
            ind->constr_violation++;
    }
}

void check_class_lim(individual *ind, problem_instance *pi)
{
    int c, s, m;
    for (c = 0; c < pi->nm_Activity; c++)
    {
        unsigned class_lim = pi->sigma_class[c];
        unsigned long class_count = 0;
        for (s = 0; s < pi->nm_Students; s++)
        {
            for (m = 0; m < pi->Cs[s].nm_courses; m++)
            {
                if (ind->student_courses[s][m] == pi->Cs[s].courses[m].id)
                {
                    t_activity *course_activities = (t_activity *)malloc(pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities * sizeof(t_activity));
                    memcpy(
                        course_activities,
                        pi->Ac[pi->Cs[s].courses[m].id - 1].activities,
                        pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities * sizeof(t_activity));

                    for (unsigned a = 0; a < pi->Ac[pi->Cs[s].courses[m].id - 1].nm_activities; ++a)
                    {

                        if (strcmp(pi->A[c].id, course_activities[a].id) == 0)
                        {
                            class_count++;
                            break;
                        }
                    }
                }
            }
        }

        if (class_count > class_lim)
            ind->constr_violation++;
    }
}

void check_min_mods(individual *ind, problem_instance *pi)
{
    int s, m;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned min_mods = pi->kmins[s];
        unsigned mods = 0;
        for (m = 0; m < pi->Cs[s].nm_courses; m++)
            if (ind->student_courses[s][m] == pi->Cs[s].courses[m].id)
                mods++;

        if (mods < min_mods)
            ind->constr_violation++;
    }
}

void check_max_mods(individual *ind, problem_instance *pi)
{
    int s, m;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned max_mods = pi->kmaxs[s];
        unsigned mods = 0;
        for (m = 0; m < pi->Cs[s].nm_courses; m++)
            if (ind->student_courses[s][m] == pi->Cs[s].courses[m].id)
                mods++;

        if (mods > max_mods)
            ind->constr_violation++;
    }
}

void test_problem(individual *ind, problem_instance *pi)
{
    /*objective functions*/
    countTimesRequestsMet(ind->student_courses, ind->gene, ind->obj, pi);
    countCourseRequestsMet(ind->student_courses, ind->obj, pi);

    /*constraint handling*/
    ind->constr_violation = 0;
    check_room_cap(ind, pi);
    check_class_lim(ind, pi);
    check_min_mods(ind, pi);
    check_max_mods(ind, pi);
}

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind, const problem_instance *pi)
{

    test_problem(ind, pi);

    return;
}
