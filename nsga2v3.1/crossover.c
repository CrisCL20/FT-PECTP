/* Crossover routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

void fill_missing_activities(problem_instance *pi, individual *parent1, individual *parent2, individual *child)
{
    int a, r;
    for (a = 0; a < pi->nm_Activity; a++)
    {
        t_cellTuple *cell = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
        act_in_ind(pi, child, pi->A[a], cell);
        // if the activity is not in the child...
        if (cell == NULL)
        {
            // see where it was in parents and try to assign it in the same cell
            int assigned = 0;
            t_cellTuple *cell_p1 = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
            act_in_ind(pi, parent1, pi->A[a], cell_p1);

            t_cellTuple *cell_p2 = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
            act_in_ind(pi, parent2, pi->A[a], cell_p2);

            if (cell_p1 != NULL)
            {
                if (strcmp(child->gene[cell_p1->r][cell_p1->t].id, EmptyActivity.id) == 0)
                {
                    child->gene[cell_p1->r][cell_p1->t] = pi->A[a];
                    assigned = 1;
                    break;
                }

                else
                {
                    for (r = 0; r < pi->Ra[a].nm_rooms; r++)
                    {
                        unsigned ridx = pi->Ra[a].rooms[r].id - 1;
                        if (strcmp(child->gene[ridx][cell_p1->t].id, EmptyActivity.id) == 0)
                        {
                            child->gene[ridx][cell_p1->t] = pi->A[a];
                            assigned = 1;
                            break;
                        }
                    }
                }
            }

            else if (cell_p2 != NULL)
            {
                if (strcmp(child->gene[cell_p2->r][cell_p2->t].id, EmptyActivity.id) == 0)
                {
                    child->gene[cell_p2->r][cell_p2->t] = pi->A[a];
                    assigned = 1;
                    break;
                }
                else
                {
                    for (r = 0; r < pi->Ra[a].nm_rooms; r++)
                    {
                        unsigned ridx = pi->Ra[a].rooms[r].id - 1;
                        if (strcmp(child->gene[ridx][cell_p2->t].id, EmptyActivity.id) == 0)
                        {
                            child->gene[ridx][cell_p2->t] = pi->A[a];
                            assigned = 1;
                            break;
                        }
                    }
                }
            }

            while (!assigned)
            {
                int r = rnd(0, pi->Ra[a].nm_rooms - 1);
                r = pi->Ra[a].rooms[r].id - 1;
                int t = rnd(0, pi->nm_TimeSlots);

                if (strcmp(child->gene[r][t].id, EmptyActivity.id) == 0)
                {
                    child->gene[r][t] = pi->A[a];
                    assigned = 1;
                }
            }

            free(cell_p1);
            free(cell_p2);
        }

        free(cell);
    }
}

void fix_course_collisions(problem_instance *pi, individual *child)
{
    int c, a;
    for (c = 0; c < pi->nm_Courses; c++)
    {
        int *tslot_used = (int *)calloc(pi->nm_TimeSlots, sizeof(int));
        for (a = 0; a < pi->Ac[c].nm_activities; a++)
        {
            t_cellTuple *act_cell = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
            act_in_ind(pi, child, pi->Ac[c].activities[a], act_cell);
            if (tslot_used[act_cell->t] == 1)
            {
                int new_tslot = (act_cell->t + 1) % pi->nm_TimeSlots, moved = 0, ts_elapsed = 0;
                while (!moved && ts_elapsed < pi->nm_TimeSlots)
                {
                    if (strcmp(child->gene[act_cell->r][new_tslot].id, EmptyActivity.id) == 0 && tslot_used[new_tslot] == 0)
                    {
                        child->gene[act_cell->r][new_tslot] = child->gene[act_cell->r][act_cell->t];
                        child->gene[act_cell->r][act_cell->t] = EmptyActivity;
                        moved = 1;
                        break;
                    }
                    new_tslot = (new_tslot + 1) % pi->nm_TimeSlots;
                    ts_elapsed++;
                }
            }
            else
                tslot_used[act_cell->t] = 1;

            free(act_cell);
        }
        free(tslot_used);
    }
}

void repair_child(problem_instance *pi, individual *parent1, individual *parent2, individual *child)
{
    fill_missing_activities(pi, parent1, parent2, child);
    fix_course_collisions(pi, child);
}

void verify_ind(problem_instance *pi, individual *ind)
{
    int a;

    for (a = 0; a < pi->nm_Activity; a++)
    {
        t_cellTuple *cell = (t_cellTuple *)malloc(sizeof(t_cellTuple));
        act_in_ind(pi, ind, pi->A[a], cell);
        if (cell == NULL)
        {
            printf("Could not find activity %s...", pi->A[a].id);
            exit(EXIT_FAILURE);
        }
        free(cell);
    }
}

/* Function to cross two individuals */
void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{

    if (randomperc() < pcross_bin)
    {

        unsigned r, t, i;

        int n_tslots_p1 = ceil(0.5 * pi->nm_TimeSlots);
        int n_tslots_p2 = pi->nm_TimeSlots - n_tslots_p1;

        int *all_tslots = (int *)calloc(pi->nm_TimeSlots, sizeof(int));
        for (i = 0; i < pi->nm_TimeSlots; i++)
            all_tslots[i] = i;

        int *tslot_idx_p1 = (int *)calloc(n_tslots_p1, sizeof(int));
        int *tslot_idx_p2 = (int *)calloc(n_tslots_p2, sizeof(int));

        for (i = 0; i < n_tslots_p1; i++)
            tslot_idx_p1[i] = all_tslots[i];
        for (i = 0; i < n_tslots_p2; i++)
            tslot_idx_p2[i] = all_tslots[i + n_tslots_p1];

        t_activity assigned_classes_p1[pi->nm_Activity];
        int count_assigned_p1 = 0;
        t_activity assigned_classes_p2[pi->nm_Activity];
        int count_assigned_p2 = 0;

        /*inicializar hijos sin ninguna clase*/
        for (r = 0; r < pi->nm_Rooms; r++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
            {
                child1->gene[r][t] = EmptyActivity;
                child2->gene[r][t] = EmptyActivity;
            }
        }

        /*crossover XVRA con 100% de los timeslots heredados*/

        /*
        Child 1
        */
        /*get genetics from p1*/
        for (t = 0; t < n_tslots_p1; t++)
        {
            for (r = 0; r < pi->nm_Rooms; r++)
            {
                child1->gene[r][tslot_idx_p1[t]] = parent1->gene[r][tslot_idx_p1[t]];
                if (strcmp(child1->gene[r][tslot_idx_p1[t]].id, EmptyActivity.id) != 0)
                    assigned_classes_p1[count_assigned_p1++] = child1->gene[r][tslot_idx_p1[t]];
            }
        }
        /*get genetics from p2*/
        for (t = 0; t < n_tslots_p2; t++)
        {
            for (r = 0; r < pi->nm_Rooms; r++)
            {
                child1->gene[r][tslot_idx_p2[t]] = parent2->gene[r][tslot_idx_p2[t]];
                for (i = 0; i < count_assigned_p1; i++)
                {
                    if (strcmp(parent2->gene[r][tslot_idx_p2[t]].id, assigned_classes_p1[i].id) == 0)
                        child1->gene[r][tslot_idx_p2[t]] = EmptyActivity;
                }
            }
        }

        /*place missing activities in child 1*/
        repair_child(pi, parent1, parent2, child1);

        /*
        Child 2
        */
        /*get genetics from p2*/
        for (t = 0; t < n_tslots_p2; t++)
        {
            for (r = 0; r < pi->nm_Rooms; r++)
            {
                child2->gene[r][tslot_idx_p2[t]] = parent2->gene[r][tslot_idx_p2[t]];
                if (strcmp(child2->gene[r][tslot_idx_p2[t]].id, EmptyActivity.id) != 0)
                    assigned_classes_p2[count_assigned_p2++] = child2->gene[r][tslot_idx_p2[t]];
            }
        }
        /*get genetics from p1*/
        for (t = 0; t < n_tslots_p1; t++)
        {
            for (r = 0; r < pi->nm_Rooms; r++)
            {
                child2->gene[r][tslot_idx_p1[t]] = parent1->gene[r][tslot_idx_p1[t]];
                for (i = 0; i < count_assigned_p2; i++)
                {
                    if (strcmp(parent1->gene[r][tslot_idx_p1[t]].id, assigned_classes_p2[i].id) == 0)
                        child2->gene[r][tslot_idx_p1[t]] = EmptyActivity;
                }
            }
        }

        /*place missing activities in child 2*/
        repair_child(pi, parent2, parent1, child2);

        free(tslot_idx_p1);
        free(tslot_idx_p2);
        free(all_tslots);
    }

    else
    {
        child1 = parent1;
        child2 = parent2;
    }
}