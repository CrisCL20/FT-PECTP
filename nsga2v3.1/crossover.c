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

void inherit_parents(problem_instance* pi, individual* parent1, individual* parent2, individual* child, size_t n_courses_p1, size_t n_courses_p2, size_t* courses_p1, size_t* courses_p2){

    // inherit courses from p1 to ch1
    int c;
    for (int r = 0; r < pi->nm_Rooms; r++)
        for (int t = 0; t < pi->nm_TimeSlots; t++)
            child->gene[r][t] = EmptyActivity;

    for (c = 0; c < n_courses_p1; c++){
        for (int a = 0; a < pi->Ac[courses_p1[c]].nm_activities; a++){
            t_cellTuple* act_cell = (t_cellTuple *) calloc(1,sizeof(t_cellTuple));
            act_in_ind(pi,parent1,pi->Ac[courses_p1[c]].activities[a],act_cell);
            if (act_cell != NULL)
                child->gene[act_cell->r][act_cell->t] = pi->Ac[courses_p1[c]].activities[a];
            free(act_cell);
        }
    }

    // fill courses of p2 to ch1
    for (c = 0; c < n_courses_p2; c++) {
        for (int a = 0; a < pi->Ac[courses_p2[c]].nm_activities; a++) {
            t_cellTuple* act_cell = (t_cellTuple *) calloc(1,sizeof(t_cellTuple));
            act_in_ind(pi,parent2,pi->Ac[courses_p2[c]].activities[a],act_cell);
            if (act_cell != NULL){
                if (strcmp(child->gene[act_cell->r][act_cell->t].id, EmptyActivity.id) == 0) {
                    child->gene[act_cell->r][act_cell->t] = pi->Ac[courses_p2[c]].activities[a];
                }

                else {
                    size_t act_idx = get_act_idx(pi,pi->Ac[courses_p2[c]].activities[a]);
                    int assigned = 0;
                    for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++){
                        if (strcmp(child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell->t].id, EmptyActivity.id) == 0){
                            child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell->t] = pi->Ac[courses_p2[c]].activities[a];
                            assigned = 1;
                            break;
                        }
                    }

                    if (!assigned) {
                        for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++) {
                            for (int t = 0; t < pi->nm_TimeSlots; t++) {
                                if (strcmp(child->gene[pi->Ra[act_idx].rooms[r].id - 1][t].id, EmptyActivity.id) == 0){
                                    child->gene[pi->Ra[act_idx].rooms[r].id - 1][t] = pi->Ac[courses_p2[c]].activities[a];
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            free(act_cell);
        }
    }

}

/* Function to cross two individuals */
void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{

    if (randomperc() < pcross_bin)
    {
        size_t n_courses_p1 = (size_t) ceil(pi->nm_Courses * 0.5);
        size_t n_courses_p2 = pi->nm_Courses - n_courses_p1;

        size_t courses_p1[n_courses_p1];
        size_t courses_p2[n_courses_p2];

        int * all_courses = (int *) calloc(pi->nm_Courses, sizeof(int));
        for (int i = 0; i < pi->nm_Courses; i++)
            all_courses[i] = pi->C[i].id - 1;

        shuffle(all_courses, pi->nm_Courses);

        size_t c;
        for (c = 0; c < n_courses_p1; c++)
            courses_p1[c] = all_courses[c];
        for (c = 0; c < n_courses_p2; c++)
            courses_p2[c] = all_courses[c + n_courses_p1];

        inherit_parents(pi,parent1,parent2,child1,n_courses_p1,n_courses_p2,courses_p1,courses_p2);
        inherit_parents(pi,parent2,parent1,child2,n_courses_p2,n_courses_p1,courses_p2,courses_p1);
        verify_ind(pi,child1);
        verify_ind(pi,child2);

        free(all_courses);
    }

    else
    {
        child1 = parent1;
        child2 = parent2;
    }
}