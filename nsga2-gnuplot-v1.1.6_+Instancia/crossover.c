/* Crossover routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

void assign_unique_block_indices(int *tslot_idx_p1, int *tslot_idx_p2, int T, int n_tslots)
{
    int i;
    int *all = malloc(T * sizeof(int));
    for (i = 0; i < T; i++)
        all[i] = i;

    shuffle(all, T);

    /*Asignar sin solapamiento*/
    for (i = 0; i < n_tslots; i++)
    {
        tslot_idx_p1[i] = all[i];
        tslot_idx_p2[i] = all[i + n_tslots];
    }

    free(all);
}

/* Function to cross two individuals */
void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{
    unsigned r, t, tp, i;

    int n_tslots = floor(pi->nm_TimeSlots / 3) + 1;

    int *tslot_idx_p1 = malloc(n_tslots * sizeof(int));
    int *tslot_idx_p2 = malloc(n_tslots * sizeof(int));

    assign_unique_block_indices(tslot_idx_p1, tslot_idx_p2, pi->nm_TimeSlots, n_tslots);

    t_activity assigned_classes_p1[pi->nm_Activity];
    int count_assigned_p1 = 0;
    t_activity assigned_classes_p2[pi->nm_Activity];
    int count_assigned_p2 = 0;

    int *not_used_tslots_idx = malloc((pi->nm_TimeSlots - n_tslots) * sizeof(int));
    int count_not_used_tslots = 0;
    /*inicializar hijos sin ninguna clase*/
    for (r = 0; r < pi->nm_Rooms; r++)
    {
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            child1->gene[r][t] = EmptyActivity;
            child1->gene[r][t] = EmptyActivity;
        }
    }

    /*conjunto de tslots no usados*/
    for (t = 0; t < pi->nm_TimeSlots; t++)
    {
        int used = 0;
        for (tp = 0; tp < n_tslots; tp++)
            if (t == tslot_idx_p1[tp] || t == tslot_idx_p2[tp])
            {
                used = 1;
                break;
            }
        if (!used)
            not_used_tslots_idx[count_not_used_tslots++] = t;
    }

    /*crossover*/
    for (r = 0; r < pi->nm_Rooms; r++)
    {
        for (t = 0; t < n_tslots; t++)
        {
            int t1 = tslot_idx_p1[t];
            int t2 = tslot_idx_p2[t];

            /*p1 asistido de p2*/

            child1->gene[r][t1] = parent1->gene[r][t1];
            if (cmpactivity(child1->gene[r][t1], EmptyActivity) != 0)
                assigned_classes_p1[count_assigned_p1++] = child1->gene[r][t1];

            child1->gene[r][t2] = parent2->gene[r][t2];
            /*checkear si hay una clase en p2 que ya fue asignada por p1*/
            for (i = 0; i < count_assigned_p1; i++)
            {
                if (cmpactivity(parent2->gene[r][t2], assigned_classes_p1[i]) == 0)
                    /*si la clase fue asignada por p1, hacer que el cromosoma no tenga la clase duplicada*/
                    child1->gene[r][t2] = EmptyActivity;
            }

            /*p2 asistido de p1*/
            child2->gene[r][t1] = parent1->gene[r][t1];
            /*checkear si hay una clase en p1 que ya fue asignada por p2*/
            for (i = 0; i < count_assigned_p2; i++)
            {
                if (cmpactivity(parent1->gene[r][t1], assigned_classes_p2[i]) == 0)
                    /*si la clase fue asignada por p2, hacer que el cromosoma no tenga la clase duplicada*/
                    child2->gene[r][t1] = EmptyActivity;
            }

            child2->gene[r][t2] = parent2->gene[r][t2];
            if (cmpactivity(child2->gene[r][t2], EmptyActivity) != 0)
                assigned_classes_p2[count_assigned_p2++] = child2->gene[r][t2];
        }
    }

    /*calcular conjunto de clases no asignadas*/
    unsigned unassigned_classes_c1[pi->nm_Activity];
    int count_unassigned_c1 = 0;
    unsigned unassigned_classes_c2[pi->nm_Activity];
    int count_unassigned_c2 = 0;

    unsigned c;

    for (c = 0; c < pi->nm_Activity; c++)
    {
        int class_assigned_c1 = 0, class_assigned_c2 = 0;
        for (r = 0; r < pi->nm_Rooms; r++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
            {
                if (cmpactivity(child1->gene[r][t], pi->A[c]) == 0)
                    class_assigned_c1 = 1;
                if (cmpactivity(child2->gene[r][t], pi->A[c]) == 0)
                    class_assigned_c2 = 1;
                if (class_assigned_c1 && class_assigned_c2)
                    break;
            }
            /*dejar de buscar si ya se encontro la clase*/
            if (class_assigned_c1 && class_assigned_c2)
                break;
        }

        if (class_assigned_c1 == 0)
        {
            unassigned_classes_c1[count_unassigned_c1++] = c;
        }
        if (class_assigned_c2 == 0)
        {
            unassigned_classes_c2[count_unassigned_c2++] = c;
        }
    }

    /*asignar clases faltantes a c1 y c2*/
    unsigned c1, c2;

    for (t = 0; t < count_not_used_tslots; t++)
    {
        for (c1 = 0; c1 < count_unassigned_c1; c1++)
        {
            // @TODO: need a way to find activity index for unassigned_classes_c1[c1]
            // maybe bsearch??

            for (r = 0; r < pi->Ra[unassigned_classes_c1[c1]].nm_rooms; r++)
            {
                unsigned ridx = pi->Ra[unassigned_classes_c1[c1]].rooms[r].id - 1;
                if (cmpactivity(child1->gene[ridx][not_used_tslots_idx[t]], EmptyActivity) == 0)
                {
                    child1->gene[ridx][not_used_tslots_idx[t]] = pi->A[unassigned_classes_c1[c1]];
                    break;
                }
            }
        }

        for (c2 = 0; c2 < count_unassigned_c2; c2++)
        {
            for (r = 0; r < pi->Ra[unassigned_classes_c2[c2]].nm_rooms; r++)
            {
                unsigned ridx = pi->Ra[unassigned_classes_c2[c2]].rooms[r].id - 1;
                if (cmpactivity(child2->gene[ridx][not_used_tslots_idx[t]], EmptyActivity) == 0)
                {
                    child2->gene[ridx][not_used_tslots_idx[t]] = pi->A[unassigned_classes_c2[c2]];
                    break;
                }
            }
        }
    }

    assign_students(child1, pi);
    assign_students(child2, pi);

    free(tslot_idx_p1);
    free(tslot_idx_p2);
    free(not_used_tslots_idx);
}