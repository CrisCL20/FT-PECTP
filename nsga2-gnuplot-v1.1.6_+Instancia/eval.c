/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        evaluate_ind(&(pop->ind[i]), pi);
    }
    return;
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

/*FO1: preferencias horarias*/
void countTimesRequestsMet(unsigned **student_schedule, t_activity **gene, double *obj, problem_instance *pi)
{
    int i, j, k, r, t;
    unsigned long counts = 0;

    for (i = 0; i < pi->nm_Students; i++)
    {
        /*count where the student goes to classes in a time period they did not request*/

        for (j = 0; j < pi->Cs[i].nm_courses; ++j)
        {
            if (student_schedule[i][j])
            {
                size_t c_idx = pi->Cs[i].courses[j].id - 1;
                for (k = 0; k < pi->Ac[c_idx].nm_activities; k++)
                {
                    int found_act = 0;
                    for (r = 0; r < pi->nm_Rooms; ++r)
                    {
                        for (t = 0; t < pi->Ts[i].nm_timeslots; t++)
                        {
                            int ts_idx = get_timeslot_idx(pi, pi->Ts[i].timeslots[t]);
                            if (ts_idx != -1 && strcmp(gene[r][ts_idx].id, pi->Ac[c_idx].activities[k].id) == 0)
                            {
                                found_act = 1;
                                counts++;
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

    obj[0] = counts;
}

/*FO2: cantidad de modulos preferidos no asignados*/
void countCourseRequestsMet(unsigned **students_schedule, double *obj, problem_instance *pi)
{
    int i, j;
    unsigned long counts = 0;

    for (i = 0; i < pi->nm_Students; i++)
    {
        unsigned met = 0;
        for (j = 0; j < pi->Cs[i].nm_courses; j++)
            met += students_schedule[i][j];

        // printf("Request met for student %d: %d\n ", pi->S[i].id, met);

        counts += pi->Cs[i].nm_courses - met;
    }

    // printf("\nObjetivo 2: %ld\n", counts);
    // exit(0);

    obj[1] = counts;
}

void check_room_cap(individual *ind, problem_instance *pi)
{
    int r, s, t;

    for (r = 0; r < pi->nm_Rooms; r++)
    {
        int room_cap = pi->rho[r];

        // see that in each timeslot, the room does not have more than pi->rho[r] students, if it does, decrease constr_violation
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            int n_students = 0;
            t_activity act = ind->gene[r][t];

            // if the cell is empty, go to next timeslot
            if (strcmp(act.id, EmptyActivity.id) == 0)
                continue;

            int parent_course_id = get_course_activity(pi, act);

            for (s = 0; s < pi->nm_Students; s++)
            {
                // if the student is enrolled in activity parent course, then he assists to the activity
                int course_idx_in_student_pref = course_in_student_preference(pi, s, parent_course_id);
                if (course_idx_in_student_pref != -1 && ind->student_courses[s][course_idx_in_student_pref])
                    n_students++;
            }

            if (n_students > room_cap)
                ind->constr_violation--;
        }
    }
}

void check_course_lim(individual *ind, problem_instance *pi)
{
    int c, s, aux_c;
    for (c = 0; c < pi->nm_Courses; c++)
    {
        unsigned course_lim = pi->sigma_class[c];
        unsigned long course_count = 0;
        for (s = 0; s < pi->nm_Students; s++)
            for (aux_c = 0; aux_c < pi->Cs[s].nm_courses; aux_c++)
                if (ind->student_courses[s][aux_c] && pi->Cs[s].courses[aux_c].id == pi->C[c].id)
                    course_count++;

        if (course_count > course_lim)
            ind->constr_violation--;
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
            if (ind->student_courses[s][m])
                mods++;

        if (mods < min_mods)
            ind->constr_violation--;
    }
}

void check_max_mods(individual *ind, problem_instance *pi)
{
    int s, m;
    unsigned mods = 0;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned max_mods = pi->kmaxs[s];
        for (m = 0; m < pi->Cs[s].nm_courses; m++)
            if (ind->student_courses[s][m])
                mods++;

        if (mods > max_mods)
            ind->constr_violation--;
        mods = 0;
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
    check_course_lim(ind, pi);
    check_min_mods(ind, pi);
    check_max_mods(ind, pi);
}

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind, problem_instance *pi)
{

    test_problem(ind, pi);

    return;
}
