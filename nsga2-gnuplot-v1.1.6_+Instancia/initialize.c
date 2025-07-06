/* Data initializtion routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

typedef struct
{
    unsigned mid;
    unsigned degree;
} mod_prios;

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

void set_modules_matrix(individual *ind, unsigned **mat, problem_instance *pi)
{
    int i, j, r, t;

    for (i = 0; i < pi->M; i++)
    {
        unsigned c1m_i = 2 * (i + 1) - 1, c2m_i = 2 * (i + 1);
        /*finding timeslot index for c1 and c2*/
        int c1_ts_idx = -1, c2_ts_idx = -1;
        for (r = 0; r < pi->R; r++)
        {
            for (t = 0; t < pi->T; t++)
            {
                if (ind->gene[r][t] == c1m_i)
                    c1_ts_idx = t;
                else if (ind->gene[r][t] == c2m_i)
                    c2_ts_idx = t;
                /*break when both classes have been found*/
                if (c1_ts_idx != -1 && c2_ts_idx != -1)
                    break;
            }
        }

        /*if both classes clash, then m_i should be penalized*/
        if (c1_ts_idx == c2_ts_idx)
            mat[i][i] = 999;

        /*find classes that clash with c1 and c2*/
        int clash_c1_ = 0, clash_c2_ = 0;
        unsigned *clashes_with_c1 = (unsigned *)malloc((pi->R - 1) * sizeof(unsigned));
        unsigned *clashes_with_c2 = (unsigned *)malloc((pi->R - 1) * sizeof(unsigned));
        for (r = 0; r < pi->R; r++)
        {
            if (ind->gene[r][c1_ts_idx] != 0 && ind->gene[r][c1_ts_idx] != c1m_i)
                clashes_with_c1[clash_c1_++] = ind->gene[r][c1_ts_idx];

            if (ind->gene[r][c2_ts_idx] != 0 && ind->gene[r][c2_ts_idx] != c2m_i)
                clashes_with_c2[clash_c2_++] = ind->gene[r][c2_ts_idx];
        }

        /*get modules that clash with m_i*/
        for (j = 0; j < clash_c1_; j++)
        {
            int m_j = (clashes_with_c1[j] - 1) / 2;
            mat[i][m_j] = 1;
        }

        for (j = 0; j < clash_c2_; j++)
        {
            int m_j = (clashes_with_c2[j] - 1) / 2;
            mat[i][m_j] = 1;
        }

        free(clashes_with_c1);
        free(clashes_with_c2);
    }
}

int comp(const void *a, const void *b)
{
    return ((mod_prios *)a)->degree - ((mod_prios *)b)->degree;
}

void assign_students(individual *ind, problem_instance *pi)
{
    int i, j;
    unsigned **mod_mat = (unsigned **)malloc(pi->M * sizeof(unsigned *));
    for (i = 0; i < pi->M; i++)
        mod_mat[i] = (unsigned *)malloc(pi->M * sizeof(unsigned));
    for (i = 0; i < pi->M; i++)
        for (j = 0; j < pi->M; j++)
            mod_mat[i][j] = 0;

    set_modules_matrix(ind, mod_mat, pi);

    int s, midx;
    for (s = 0; s < pi->S; s++)
    {
        /*get total clashes per module*/
        mod_prios *priorities = (mod_prios *)malloc(pi->mod_prefs[s].nmods * sizeof(mod_prios));
        for (midx = 0; midx < pi->mod_prefs[s].nmods; midx++)
        {
            unsigned *clashes_arr = mod_mat[pi->mod_prefs[s].mods[midx].id - 1];
            priorities[midx].mid = pi->mod_prefs[s].mods[midx].id;
            priorities[midx].degree = 0;
            for (i = 0; i < pi->M; i++)
                priorities[midx].degree += clashes_arr[i];
        }

        /**************/
        /*do the greedy lol*/
        /**************/

        qsort(priorities, pi->mod_prefs[s].nmods, sizeof(mod_prios), comp);

        int *assigned = (int *)calloc(pi->mod_prefs[s].nmods, sizeof(int));
        int count_assigned = 0;

        for (midx = 0; midx < pi->mod_prefs[s].nmods; midx++)
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
            if (!conflict && priorities[midx].degree <= pi->M)
            {
                assigned[count_assigned++] = m_jidx;
                ind->student_modules[s][midx] = 1;
            }
        }

        free(assigned);
        free(priorities);
    }

    for (i = 0; i < pi->M; i++)
        free(mod_mat[i]);
    free(mod_mat);
}

/* Function to initialize an individual randomly */
void initialize_ind(individual *ind, problem_instance *pi)
{
    int i, j;

    for (i = 0; i < pi->R; i++)
        for (j = 0; j < pi->T; j++)
            ind->gene[i][j] = 0;

    for (i = 0; i < pi->C; i++)
    {
        /*asignar la clase i a un salón y timeslot aleatorio*/

        while (1)
        {
            /*asegurarnos que el salón elegido sea adecuado para la clase*/
            int room_choice_idx = rnd(0, pi->adqte_rooms[i].nrooms - 1);
            unsigned rid = pi->adqte_rooms[i].rooms[room_choice_idx].id;

            int ts_choice_idx = rnd(0, pi->T - 1);

            if (ind->gene[rid - 1][ts_choice_idx] == 0)
            {
                ind->gene[rid - 1][ts_choice_idx] = pi->classes[i].id;
                break;
            }
        }
    }

    assign_students(ind, pi);

    return;
}
