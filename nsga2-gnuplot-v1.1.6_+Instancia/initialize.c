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

void set_modules_matrix(individual *ind, unsigned **mat, problem_instance *pi)
{
    int *act_to_ts = malloc(pi->nm_Activity * sizeof(int));
    for (int i = 0; i < pi->nm_Activity; i++)
        act_to_ts[i] = -1;

    for (int r = 0; r < pi->nm_Rooms; r++)
    {
        for (int t = 0; t < pi->nm_TimeSlots; t++)
        {
            if (strcmp(ind->gene[r][t].id, EmptyActivity.id) != 0)
            {
                int a_idx = get_act_idx(pi, ind->gene[r][t]);
                if (a_idx != -1)
                    act_to_ts[a_idx] = t;
            }
        }
    }

    // 2. Llenado de matriz de colisiones entre cursos
    for (int i = 0; i < pi->nm_Courses; i++)
    {
        for (int a_idx = 0; a_idx < pi->Ac[i].nm_activities; a_idx++)
        {
            int current_act = get_act_idx(pi, pi->Ac[i].activities[a_idx]);
            int t1 = act_to_ts[current_act];
            if (t1 == -1)
                continue;

            // Revisar qué otras cosas hay en ese mismo bloque t1 en otros salones
            for (int r = 0; r < pi->nm_Rooms; r++)
            {
                if (strcmp(ind->gene[r][t1].id, EmptyActivity.id) != 0 &&
                    strcmp(ind->gene[r][t1].id, pi->Ac[i].activities[a_idx].id) != 0)
                {

                    int other_course = find_course(pi, ind->gene[r][t1]);
                    mat[i][other_course] = 1;
                }
            }
        }
    }
    free(act_to_ts);
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
    int i, j;
    unsigned **mod_mat = (unsigned **)malloc(pi->nm_Courses * sizeof(unsigned *));
    for (i = 0; i < pi->nm_Courses; i++)
        mod_mat[i] = (unsigned *)calloc(pi->nm_Courses, sizeof(unsigned));

    // printf("==== Matriz de colisiones ====\n");
    set_modules_matrix(ind, mod_mat, pi);
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
            priorities[midx].mid = pi->Cs[s].courses[midx].id;
            priorities[midx].degree = 0;

            unsigned course_conflict = 0;
            for (i = 0; i < pi->nm_Courses; i++)
                course_conflict += mod_mat[course_idx][i];

            // add free time conflict penalization
            unsigned freetime_conflict = 0;
            for (a = 0; a < pi->Ac[course_idx].nm_activities; a++)
            {
                int act_ts = find_activity_timeslot(pi, ind, pi->Ac[course_idx].activities[a]);
                if (act_ts != -1 && timeslot_in_student_preference(pi, s, pi->T[act_ts]))
                    freetime_conflict += FREETIME_PENALIZATION;
            }

            priorities[midx].degree += freetime_conflict;
        }

        /**************/
        /*do the greedy lol*/
        /**************/

        // sort priorities by degree
        qsort(priorities, pi->Cs[s].nm_courses, sizeof(course_prios), comp);

        int *assigned = (int *)calloc(pi->Cs[s].nm_courses, sizeof(int));
        int count_assigned = 0;

        int _priorities_idx = 0;
        for (midx = 0; midx < pi->Cs[s].nm_courses; midx++)
        {
            /*select module with least degree*/
            unsigned least_conflicted_idx = priorities[_priorities_idx].mid - 1;

            /* is there any problem in assigning current least_conflicted with any of the previous assignations? */
            int conflict = 0;
            for (j = 0; j < count_assigned; j++)
            {
                if (mod_mat[least_conflicted_idx][assigned[j]])
                {
                    conflict = 1;
                    break;
                }
            }

            /*assign module if it has no conflict*/
            if (!conflict && priorities[_priorities_idx].degree <= pi->nm_Courses)
            {
                assigned[count_assigned++] = least_conflicted_idx;
                // this write should be on the idx in student_courses rather than the global idx
                // ex: least_conflicted=C13 but in the student array that could correspond to idx 2
                int _idx = find_idx_in_student_preference(pi, s, priorities[_priorities_idx].mid);
                ind->student_courses[s][_idx] = 1;
            }
            _priorities_idx++;
        }

        free(assigned);
        // free(priorities);
    }

    for (i = 0; i < pi->nm_Courses; i++)
        free(mod_mat[i]);
    free(mod_mat);
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
