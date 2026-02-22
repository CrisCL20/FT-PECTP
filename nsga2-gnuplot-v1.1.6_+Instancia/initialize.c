/* Data initializtion routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

#define PENALIZATION 1E6
#define FREETIME_PENALIZATION 1E2

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

void set_modules_penalties(problem_instance *pi, individual *ind, unsigned *mat, int *act_ts)
{
    // Get map of activities to timeslots
    for (int a = 0; a < pi->nm_Activity; a++)
    {
        for (int r = 0; r < pi->nm_Rooms; r++)
        {
            int found_act = 0;
            for (int t = 0; t < pi->nm_TimeSlots; t++)
            {
                if (strcmp(ind->gene[r][t].id, pi->A[a].id) == 0)
                {
                    act_ts[a] = t;
                    found_act = 1;
                }
            }
            if (found_act)
                break;
        }
    }

    // penalizar cursos que tengan topes consigo mismos
    for (int c = 0; c < pi->nm_Courses; c++)
    {
        int penalized = 0;
        for (int a_c = 0; a_c < pi->Ac[c].nm_activities; a_c++)
        {
            int current_act_idx = get_act_idx(pi, pi->Ac[c].activities[a_c]);
            int t = act_ts[current_act_idx];
            // ver si hay otra actividad del curso c en el bloque t
            for (int aa_c = 0; aa_c < pi->Ac[c].nm_activities; aa_c++)
            {
                if (aa_c != a_c && act_ts[get_act_idx(pi, pi->Ac[c].activities[aa_c])] == t)
                {
                    mat[c] = PENALIZATION;
                    break;
                }
            }

            if (penalized)
                break;
        }
    }
}

int comp(const void *a, const void *b)
{
    return (*(course_prios *)a).degree - (*(course_prios *)b).degree;
}

size_t find_idx_in_student_preference(problem_instance *pi, size_t s_idx, size_t course_id)
{
    size_t c;
    for (c = 0; c < pi->Cs[s_idx].nm_courses; ++c)
        if (pi->Cs[s_idx].courses[c].id == course_id)
            return c;

    return 0;
}

int find_activity_timeslot(problem_instance *pi, individual *ind, t_activity activity)
{
    int r, t;

    for (r = 0; r < pi->nm_Rooms; r++)
        for (t = 0; t < pi->nm_TimeSlots; t++)
            if (strcmp(ind->gene[r][t].id, activity.id) == 0)
                return t;

    return -1;
}

void assign_students(individual *ind, problem_instance *pi)
{
    unsigned *course_penalties = (unsigned *)calloc(pi->nm_Courses, sizeof(unsigned *));
    int *acts_ts = (int *)calloc(pi->nm_Activity, sizeof(int));

    // printf("==== Matriz de colisiones ====\n");
    set_modules_penalties(pi, ind, course_penalties, acts_ts);
    // printCourseMatrix(mod_mat, pi);
    // printf("==== Matriz de colisiones ====\n");

    int s, midx, a;
    for (s = 0; s < pi->nm_Students; s++)
    {
        /*get total clashes per course*/
        course_prios priorities[pi->Cs[s].nm_courses];
        for (midx = 0; midx < pi->Cs[s].nm_courses; midx++)
        {
            size_t course_idx = pi->Cs[s].courses[midx].id - 1;

            // add free time conflict penalization
            unsigned freetime_conflict = 0;
            for (a = 0; a < pi->Ac[course_idx].nm_activities; a++)
            {
                int ts_act = acts_ts[get_act_idx(pi, pi->Ac[course_idx].activities[a])];
                if (ts_act != -1 && timeslot_in_student_preference(pi, s, pi->T[ts_act]))
                    freetime_conflict += FREETIME_PENALIZATION;
            }

            priorities[midx] = (course_prios){
                .mid = pi->Cs[s].courses[midx].id,
                .degree = freetime_conflict + course_penalties[course_idx]};
        }

        /**************/
        /*do the greedy lol*/
        /**************/
        // assign all courses that have less than FREETIME_PENALIZATION degree

        for (int c = 0; c < pi->Cs[s].nm_courses; c++)
        {
            if (priorities[c].degree <= FREETIME_PENALIZATION)
            {
                ind->student_courses[s][c] = 1;
            }
        }
    }

    free(course_penalties);
    free(acts_ts);
}

void printInd(individual *ind, problem_instance *pi)
{
    size_t s, c, r, t;

    for (s = 0; s < pi->nm_Students; ++s)
    {
        printf("\nAsignación de cursos para el estudiante %d:\n\t", pi->S[s].id);
        for (c = 0; c < pi->Cs[s].nm_courses; ++c)
            printf("%d ", ind->student_courses[s][c]);
    }
    printf("\n");

    for (r = 0; r < pi->nm_Rooms; ++r)
        for (t = 0; t < pi->nm_TimeSlots; ++t)
            if (strcmp(ind->gene[r][t].id, EmptyActivity.id) != 0)
                printf("ACTIVIDAD %s EN EL SALON %d EN EL BLOQUE %s\n", ind->gene[r][t].id, pi->R[r].id, pi->T[t].ts);
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
            const int r = rnd(0, pi->nm_Rooms - 1);
            // const unsigned rid = pi->R[room_choice_idx].id;

            int ts_choice_idx = rnd(0, pi->nm_TimeSlots - 1);

            if (strcmp(ind->gene[r][ts_choice_idx].id, EmptyActivity.id) == 0)
            {
                ind->gene[r][ts_choice_idx] = pi->A[i];
                strcpy(ind->gene[r][ts_choice_idx].id, pi->A[i].id);
                break;
            }
        }
    }

    // printf("======================= ANTES DE ASIGNACION ESTUDIANTIL ==========================\n");
    // printInd(ind, pi);

    // printf("======================= =============================== ==========================\n");

    assign_students(ind, pi);

    // printf("========================================================\n");
    // printInd(ind, pi);
    // printf("========================================================\n");
    // exit(0);

    return;
}
