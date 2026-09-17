/* Crossover routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#include "global.h"
#include "rand.h"

void inherit_parents(problem_instance* pi, individual* parent1, individual* parent2, individual* child, size_t n_courses_p1, size_t n_courses_p2, size_t* courses_p1, size_t* courses_p2){

    // inherit courses from p1 to ch1
    int c;
    for (int r = 0; r < pi->nm_Rooms; r++)
        for (int t = 0; t < pi->nm_TimeSlots; t++)
            child->gene[r][t] = EMPTY_ACT;

    t_cellTuple act_cell;

    int** busy_r_t = (int **) calloc(pi->nm_Rooms, sizeof(int*));
    for (int r = 0; r < pi->nm_Rooms; r++)
       busy_r_t[r] = (int *) calloc(pi->nm_TimeSlots, sizeof(int));
    for (c = 0; c < n_courses_p1; c++){
        for (int a = 0; a < pi->Ac[courses_p1[c]].nm_activities; a++){
            if (act_in_ind(pi,parent1,pi->Ac[courses_p1[c]].activities[a],&act_cell)) {
                child->gene[act_cell.r][act_cell.t] = pi->Ac[courses_p1[c]].activity_idx[a];
                busy_r_t[act_cell.r][act_cell.t] = 1;
            }
        }
    }

    // fill courses of p2 to ch1
    for (c = 0; c < n_courses_p2; c++) {
        int *is_timeslot_free = (int *) calloc(pi->nm_TimeSlots, sizeof(int));
        for (int a = 0; a < pi->Ac[courses_p2[c]].nm_activities; a++) {
            size_t act_idx = pi->Ac[courses_p2[c]].activity_idx[a];
            act_in_ind(pi,parent2,pi->Ac[courses_p2[c]].activities[a],&act_cell);
            if (
                (child->gene[act_cell.r][act_cell.t] == EMPTY_ACT)
                && (is_timeslot_free[act_cell.t] == 0)
                && !busy_r_t[act_cell.r][act_cell.t]
            ) {
                child->gene[act_cell.r][act_cell.t] = act_idx;
                busy_r_t[act_cell.r][act_cell.t] = 1;
                is_timeslot_free[act_cell.t] = 1;
                continue;
            }

            int assigned = 0;
            /* assign to different room, same timeslot*/
            for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++){
                if (
                    child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t] == EMPTY_ACT
                    && (is_timeslot_free[act_cell.t] == 0)
                    && !busy_r_t[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t]
                ){
                    child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t] = act_idx;
                    assigned = 1;
                    busy_r_t[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t] = 1;
                    is_timeslot_free[act_cell.t] = 1;
                    break;
                }
            }

            if (assigned) continue;

            /* assign to same rooom, different timeslot */
            for (int t = 0; t < pi->nm_TimeSlots; t++) {
                if (
                    child->gene[act_cell.r][t] == EMPTY_ACT
                    && is_timeslot_free[t] == 0
                    && !busy_r_t[act_cell.r][t]
                ) {
                    child->gene[act_cell.r][t] = act_idx;
                    assigned = 1;
                    busy_r_t[act_cell.r][t] = 1;
                    is_timeslot_free[t] = 1;
                    break;
                }
            }

            if (assigned) continue;

            for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++) {
                for (int t = 0; t < pi->nm_TimeSlots; t++) {
                    if (
                        child->gene[pi->Ra[act_idx].rooms[r].id - 1][t] == EMPTY_ACT
                        && is_timeslot_free[t] == 0
                        && !busy_r_t[pi->Ra[act_idx].rooms[r].id - 1][t]
                    ){
                        child->gene[pi->Ra[act_idx].rooms[r].id - 1][t] = act_idx;
                        assigned = 1;
                        busy_r_t[pi->Ra[act_idx].rooms[r].id - 1][t] = 1;
                        is_timeslot_free[t] = 1;
                        break;
                    }
                }
                if (assigned) break;
            }
            
            if (!assigned)
                printf("Could not assign activity %s.\n", pi->Ac[courses_p2[c]].activities[a].id);
            
        }
        free(is_timeslot_free);    
    }
    
    for (int r = 0; r < pi->nm_Rooms; r++)
        free(busy_r_t[r]);
    free(busy_r_t);
    
}

void setup_courses_for_child(problem_instance* pi, t_course_sat* satisfied_demand_dom, size_t* courses_pdom, size_t* courses_psub, size_t npdom) {
    
    /* sort dominant parent satisfaction in descending order */

    int *used_courses_dom = (int *) calloc(pi->nm_Courses,sizeof(int));
    for (int c = 0; c < npdom; c++) {
        courses_pdom[c] = satisfied_demand_dom[c].cid;
        used_courses_dom[satisfied_demand_dom[c].cid] = 1;
    }

    int _idx = 0;
    for (int c = 0; c < pi->nm_Courses; c++) {
        if (!used_courses_dom[c])
            courses_psub[_idx++] = c;
    }

    free(used_courses_dom);
}

/* Function to cross two individuals */
void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{   
    if (randomperc() < pcross_bin)
    {
        size_t p = rnd(1, pi->nm_Courses - 2);

        /* procedure for child 1 */
        size_t courses_p1_ch1[p];
        size_t courses_p2_ch1[pi->nm_Courses - p];
        
        setup_courses_for_child(pi, parent1->course_sat, courses_p1_ch1, courses_p2_ch1, p);
        inherit_parents(pi, parent1, parent2, child1, p, pi->nm_Courses - p, courses_p1_ch1, courses_p2_ch1);
        
        /* procedure for child 2 */
        size_t courses_p1_ch2[pi->nm_Courses - p];
        size_t courses_p2_ch2[p];
        
        setup_courses_for_child(pi, parent2->course_sat, courses_p2_ch2, courses_p1_ch2, p);
        inherit_parents(pi,parent2, parent1, child2, p, pi->nm_Courses - p, courses_p2_ch2, courses_p1_ch2);

    }

    else
    {
        for (int r = 0; r < pi->nm_Rooms; r++) 
        {
            for (int t = 0; t < pi->nm_TimeSlots; t++)
            {
                child1->gene[r][t] = parent1->gene[r][t];
                child2->gene[r][t] = parent2->gene[r][t];
            }
        }
    }
    
}