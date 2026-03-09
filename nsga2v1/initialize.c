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
            const unsigned r = pi->Ra[i].rooms[room_choice_idx].id - 1;

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

    // printf("========================================================\n");
    // printInd(ind, pi);
    // printf("========================================================\n");
    // exit(0);

    return;
}
