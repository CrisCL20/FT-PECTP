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
    int* act_to_room = (int *) calloc(pi->nm_Activity, sizeof(int));
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
                    act_to_room[a] = r;
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

            int can_enroll = 1;
            for (int a = 0; a < pi->Ac[course_id].nm_activities; a++)
            {
                int ts = act_to_ts[get_act_idx(pi, pi->Ac[course_id].activities[a])];
                int r = act_to_room[get_act_idx(pi, pi->Ac[course_id].activities[a])];

                if (student_busy[s][ts] == 1 || course_fill[course_id] >= pi->rho[r])
                {
                    can_enroll = 0;
                    break;
                }
            }

            can_enroll = sum_array(student_courses[s], pi->Cs[s].nm_courses) <= pi->kmaxs[s];

            if (!can_enroll)
                continue;

            student_courses[s][c_idx] = 1; 
            course_fill[course_id]++;

            for (int a = 0; a < pi->Ac[course_id].nm_activities; a++)
            {
                int ts = act_to_ts[get_act_idx(pi, pi->Ac[course_id].activities[a])];
                student_busy[s][ts] = 1;
            }
        
        }
    }
    free(act_to_room);
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

/*FO1: preferencias horarias*/
void countTimesRequestsMet(int *act_to_ts, int **student_schedule, t_activity **gene, double *obj, problem_instance *pi)
{
    long double mean_insatisfaction = .0;
    for (int s = 0; s < pi->nm_Students; s++)
    {
        unsigned counts = 0;
        for (int c_idx = 0; c_idx < pi->Cs[s].nm_courses; c_idx++)
        {
            if (!student_schedule[s][c_idx]) continue;
            
            
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
        // calculate unhappyness percentage
        long double alpha_s = counts / pi->Ts[s].nm_timeslots;
        mean_insatisfaction += alpha_s;
    }

    mean_insatisfaction = mean_insatisfaction / pi->nm_Students;

    obj[0] = 1 / (1 - mean_insatisfaction);
}

/*FO2: cantidad de modulos preferidos no asignados*/
void countCourseRequestsMet(int **students_schedule, double *obj, problem_instance *pi)
{
    int i, j;
    long double mean_satisfaction = .0;
    for (i = 0; i < pi->nm_Students; i++)
    {
        unsigned met = sum_array(students_schedule[i], pi->Cs[i].nm_courses);

        // printf("Request met for student %d: %d\n ", pi->S[i].id, met);

        long double beta_s = met / pi->Cs[i].nm_courses;
        mean_satisfaction += beta_s;
    }

    mean_satisfaction = mean_satisfaction / pi->nm_Students;
    // printf("\nObjetivo 2: %ld\n", counts);
    // exit(0);

    obj[1] = 1 / mean_satisfaction;
}

void check_min_mods(individual *ind, problem_instance *pi, int **student_courses)
{
    int s, m;
    for (s = 0; s < pi->nm_Students; s++)
    {
        unsigned min_mods = pi->kmins[s];
        int total_enrollments = sum_array(student_courses[s], pi->Cs[s].nm_courses);

        if (total_enrollments < min_mods)
            ind->constr_violation--;
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
    check_min_mods(ind, pi, student_courses);

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
