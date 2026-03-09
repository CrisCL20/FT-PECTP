/* Crossover routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

// get random timeslots idx of size n_tslots for both p1 and p2
void assign_unique_block_indices(int *tslot_idx_p1, int *tslot_idx_p2, int T, int n_tslots)
{
    int i;
    int *all = malloc(T * sizeof(int));
    for (i = 0; i < T; i++)
        all[i] = i;

    /*Asignar sin solapamiento*/
    shuffle(all, T);

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

    if (randomperc() < pcross_bin)
    {

        unsigned r, t, tp, i;

        int n_tslots = floor(pi->nm_TimeSlots / 3) + 1;

        int *tslot_idx_p1 = (int *)calloc(n_tslots, sizeof(int));
        int *tslot_idx_p2 = (int *)calloc(n_tslots, sizeof(int));

        assign_unique_block_indices(tslot_idx_p1, tslot_idx_p2, pi->nm_TimeSlots, n_tslots);

        t_activity assigned_classes_p1[pi->nm_Activity];
        int count_assigned_p1 = 0;
        t_activity assigned_classes_p2[pi->nm_Activity];
        int count_assigned_p2 = 0;

        int *not_used_tslots_idx = (int *)calloc((pi->nm_TimeSlots - n_tslots), sizeof(int));
        int count_not_used_tslots = 0;
        /*inicializar hijos sin ninguna clase*/
        for (r = 0; r < pi->nm_Rooms; r++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
            {
                child1->gene[r][t] = EmptyActivity;
                child2->gene[r][t] = EmptyActivity;
            }
        }

        /*conjunto de tslots no usados*/
        for (t = 0; t < pi->nm_TimeSlots; t++)
        {
            int used = 0;
            for (tp = 0; tp < n_tslots; tp++)
            {
                if (t == tslot_idx_p1[tp] || t == tslot_idx_p2[tp])
                {
                    used = 1;
                    break;
                }
            }
            if (used == 0)
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

                strcpy(child1->gene[r][t1].id, parent1->gene[r][t1].id);
                if (strcmp(child1->gene[r][t1].id, EmptyActivity.id) != 0)
                    strcpy(assigned_classes_p1[count_assigned_p1++].id, child1->gene[r][t1].id);

                strcpy(child1->gene[r][t2].id, parent2->gene[r][t2].id);
                /*checkear si hay una clase en p2 que ya fue asignada por p1*/
                for (i = 0; i < count_assigned_p1; i++)
                {
                    if (strcmp(parent2->gene[r][t2].id, assigned_classes_p1[i].id) == 0)
                        /*si la clase fue asignada por p1, hacer que el cromosoma no tenga la clase duplicada*/
                        strcpy(child1->gene[r][t2].id, EmptyActivity.id);
                }

                /*p2 asistido de p1*/

                strcpy(child2->gene[r][t2].id, parent2->gene[r][t2].id);
                if (strcmp(child2->gene[r][t2].id, EmptyActivity.id) != 0)
                    strcpy(assigned_classes_p2[count_assigned_p2++].id, child2->gene[r][t2].id);

                strcpy(child2->gene[r][t1].id, parent1->gene[r][t1].id);
                /*checkear si hay una clase en p1 que ya fue asignada por p2*/
                for (i = 0; i < count_assigned_p2; i++)
                {
                    if (strcmp(parent1->gene[r][t1].id, assigned_classes_p2[i].id) == 0)
                        /*si la clase fue asignada por p2, hacer que el cromosoma no tenga la clase duplicada*/
                        strcpy(child2->gene[r][t1].id, EmptyActivity.id);
                }
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
                    if (strcmp(child1->gene[r][t].id, pi->A[c].id) == 0)
                        class_assigned_c1 = 1;
                    if (strcmp(child2->gene[r][t].id, pi->A[c].id) == 0)
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

        for (c1 = 0; c1 < count_unassigned_c1; c1++)
        {
            int assigned = 0;
            // shuffle rooms to get distinct assignations each time
            int n_rooms = pi->Ra[unassigned_classes_c1[c1]].nm_rooms;
            int rooms_for_class[n_rooms];

            for (r = 0; r < n_rooms; r++)
                rooms_for_class[r] = pi->Ra[unassigned_classes_c1[c1]].rooms[r].id - 1;

            shuffle(rooms_for_class, n_rooms);

            for (r = 0; r < n_rooms; ++r)
            {
                for (t = 0; t < count_not_used_tslots; t++)
                {
                    if (strcmp(child1->gene[rooms_for_class[r]][not_used_tslots_idx[t]].id, EmptyActivity.id) == 0)
                    {
                        assigned = 1;
                        strcpy(child1->gene[rooms_for_class[r]][not_used_tslots_idx[t]].id, pi->A[unassigned_classes_c1[c1]].id);
                        break;
                    }
                }
                if (assigned)
                    break;
            }
        }

        for (c2 = 0; c2 < count_unassigned_c2; c2++)
        {
            int assigned = 0;
            // shuffle rooms to get distinct assignations each time
            int n_rooms = pi->Ra[unassigned_classes_c2[c2]].nm_rooms;
            int rooms_for_class[n_rooms];

            for (r = 0; r < n_rooms; r++)
                rooms_for_class[r] = pi->Ra[unassigned_classes_c2[c2]].rooms[r].id - 1;

            shuffle(rooms_for_class, n_rooms);

            for (r = 0; r < n_rooms; ++r)
            {
                for (t = 0; t < count_not_used_tslots; t++)
                {
                    if (strcmp(child2->gene[rooms_for_class[r]][not_used_tslots_idx[t]].id, EmptyActivity.id) == 0)
                    {
                        assigned = 1;
                        strcpy(child2->gene[rooms_for_class[r]][not_used_tslots_idx[t]].id, pi->A[unassigned_classes_c2[c2]].id);
                        break;
                    }
                }
                if (assigned)
                    break;
            }
        }

        free(tslot_idx_p1);
        free(tslot_idx_p2);
        free(not_used_tslots_idx);
    }

    else
    {
        child1 = parent1;
        child2 = parent2;
    }
}