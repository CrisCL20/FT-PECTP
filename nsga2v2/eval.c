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

void assign_students(individual *ind, problem_instance *pi, int **student_courses, int **student_busy, int *course_fill, int *act_to_ts)
{
    for (int a = 0; a < pi->nm_Activity; a++)
    {
        for (int r = 0; r < pi->nm_Rooms; r++)
        {
            int found_act = 0;
            for (int t = 0; t < pi->nm_TimeSlots; t++)
            {
                if (strcmp(ind->gene[r][t].id, pi->A[a].id) == 0)
                {
                    act_to_ts[a] = t;
                    found_act = 1;
                    break;
                }
            }
            if (found_act)
                break;
        }
    }

    for (int s = 0; s < pi->nm_Students; s++)
    {

        for (int c_idx = 0; c_idx < pi->Cs[s].nm_courses; c_idx++)
        {
            int course_id = pi->Cs[s].courses[c_idx].id - 1;

            if (course_fill[course_id] >= pi->sigma_class[course_id])
                continue;

            int can_take_course = 1;
            for (int a = 0; a < pi->Ac[course_id].nm_activities; a++)
            {
                int ts = act_to_ts[get_act_idx(pi, pi->Ac[course_id].activities[a])];

                if (student_busy[s][ts] == 1)
                {
                    can_take_course = 0;
                    break;
                }
            }

            // 4. Si es factible, lo inscribimos
            if (can_take_course)
            {
                student_courses[s][c_idx] = 1; // Inscribir
                course_fill[course_id]++;      // Aumentar cupo ocupado

                // Bloquear los bloques horarios para el estudiante
                for (int a = 0; a < pi->Ac[course_id].nm_activities; a++)
                {
                    int ts = act_to_ts[get_act_idx(pi, pi->Ac[course_id].activities[a])];
                    student_busy[s][ts] = 1;
                }
            }
        }
    }
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

/*FO1: preferencias horarias*/
void countTimesRequestsMet(int *act_to_ts, int **student_schedule, t_activity **gene, double *obj, problem_instance *pi)
{
    unsigned counts = 0;

    for (int s = 0; s < pi->nm_Students; s++)
    {
        for (int c_idx = 0; c_idx < pi->Cs[s].nm_courses; c_idx++)
        {
            if (student_schedule[s][c_idx])
            {
                int c_id = pi->Cs[s].courses[c_idx].id - 1;

                for (int a = 0; a < pi->Ac[c_id].nm_activities; a++)
                {
                    int a_idx = get_act_idx(pi, pi->Ac[c_id].activities[a]);
                    int ts = act_to_ts[a_idx];

                    if (timeslot_in_student_preference(pi, s, pi->T[ts]))
                    {
                        counts += 1;
                    }
                }
            }
        }
    }
    obj[0] = counts;
}

/*FO2: cantidad de modulos preferidos no asignados*/
void countCourseRequestsMet(int **students_schedule, double *obj, problem_instance *pi)
{
    int i, j;
    unsigned long counts = 0;

    for (i = 0; i < pi->nm_Students; i++)
    {
        unsigned met = 0;
        for (j = 0; j < pi->Cs[i].nm_courses; j++)
            met += students_schedule[i][j];

        // printf("Request met for student %d: %d\n ", pi->S[i].id, met);

        counts += (pi->Cs[i].nm_courses - met);
    }

    // printf("\nObjetivo 2: %ld\n", counts);
    // exit(0);

    obj[1] = counts;
}

void check_room_cap(individual *ind, problem_instance *pi, int **student_courses)
{
    int *course_enrollment = (int *)calloc(pi->nm_Courses, sizeof(int));

    for (int s = 0; s < pi->nm_Students; s++)
    {
        for (int c_idx = 0; c_idx < pi->Cs[s].nm_courses; c_idx++)
        {
            if (student_courses[s][c_idx])
            {
                int real_course_idx = pi->Cs[s].courses[c_idx].id - 1;
                course_enrollment[real_course_idx]++;
            }
        }
    }

    for (int r = 0; r < pi->nm_Rooms; r++)
    {
        int room_cap = pi->rho[r];

        for (int t = 0; t < pi->nm_TimeSlots; t++)
        {
            t_activity act = ind->gene[r][t];

            if (strcmp(act.id, EmptyActivity.id) != 0)
            {
                int course_id = get_course_activity(pi, act) - 1;

                if (course_enrollment[course_id] > room_cap)
                    ind->constr_violation -= (course_enrollment[course_id] - room_cap);
            }
        }
    }

    free(course_enrollment);
}

void check_min_mods(individual *ind, problem_instance *pi, int **student_courses)
{
    int s, m;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned min_mods = pi->kmins[s];
        unsigned mods = 0;
        for (m = 0; m < pi->Cs[s].nm_courses; m++)
            if (student_courses[s][m])
                mods++;

        if (mods < min_mods)
            ind->constr_violation--;
    }
}

void check_max_mods(individual *ind, problem_instance *pi, int **student_courses)
{
    int s, m;
    unsigned mods = 0;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned max_mods = pi->kmaxs[s];
        for (m = 0; m < pi->Cs[s].nm_courses; m++)
            if (student_courses[s][m])
                mods++;

        if (mods > max_mods)
            ind->constr_violation--;
        mods = 0;
    }
}

void test_problem(individual *ind, problem_instance *pi)
{
    int **student_courses = (int **)calloc(pi->nm_Students, sizeof(int *));
    for (int s = 0; s < pi->nm_Students; s++)
        student_courses[s] = (int *)calloc(pi->Cs[s].nm_courses, sizeof(int));

    int **student_busy = (int **)calloc(pi->nm_Students, sizeof(int *));
    for (int s = 0; s < pi->nm_Students; s++)
        student_busy[s] = (int *)calloc(pi->nm_TimeSlots, sizeof(int));

    int *course_fill = (int *)calloc(pi->nm_Courses, sizeof(int));
    int *act_to_ts = (int *)calloc(pi->nm_Activity, sizeof(int));

    ind->obj[0] = 0;
    ind->obj[1] = 0;

    assign_students(ind, pi, student_courses, student_busy, course_fill, act_to_ts);
    /*objective functions*/
    countTimesRequestsMet(act_to_ts, student_courses, ind->gene, ind->obj, pi);
    countCourseRequestsMet(student_courses, ind->obj, pi);

    /*constraint handling*/
    ind->constr_violation = 0;
    check_room_cap(ind, pi, student_courses);
    check_min_mods(ind, pi, student_courses);
    check_max_mods(ind, pi, student_courses);

    for (int s = 0; s < pi->nm_Students; s++)
    {
        memcpy(ind->student_courses[s], student_courses[s], pi->Cs[s].nm_courses * sizeof(int));
        free(student_courses[s]);
        free(student_busy[s]);
    }
    free(student_courses);
    free(student_busy);
    free(course_fill);
    free(act_to_ts);
}

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind, problem_instance *pi)
{

    test_problem(ind, pi);

    return;
}
