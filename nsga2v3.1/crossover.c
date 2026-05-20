/* Crossover routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

typedef struct 
{
    size_t cid;
    size_t sat;
} course_demand;

int is_timeslot_free(problem_instance* pi, individual* ind, size_t t, size_t course_idx, size_t act_idx) {

    for (int a = 0; a < pi->Ac[course_idx].nm_activities; a++) {
        if (a == act_idx) continue;
        for (int r = 0; r < pi->nm_Rooms; r++) 
            if (strcmp(ind->gene[r][t].id,pi->Ac[course_idx].activities[a].id) == 0)
                return 0;
        
    }

    return 1;
}

void verify_ind(problem_instance *pi, individual *ind)
{
    int a;

    t_cellTuple cell;

    for (a = 0; a < pi->nm_Activity; a++)
    {
        
        if (act_in_ind(pi, ind, pi->A[a], &cell) == 0)
        {
            printf("Could not find activity %s...", pi->A[a].id);
            exit(EXIT_FAILURE);
        }
    }
}

void inherit_parents(problem_instance* pi, individual* parent1, individual* parent2, individual* child, size_t n_courses_p1, size_t n_courses_p2, size_t* courses_p1, size_t* courses_p2){

    // inherit courses from p1 to ch1
    int c;
    for (int r = 0; r < pi->nm_Rooms; r++)
        for (int t = 0; t < pi->nm_TimeSlots; t++)
            child->gene[r][t] = EmptyActivity;

    t_cellTuple act_cell;
    
    for (c = 0; c < n_courses_p1; c++){
        for (int a = 0; a < pi->Ac[courses_p1[c]].nm_activities; a++){
            if (act_in_ind(pi,parent1,pi->Ac[courses_p1[c]].activities[a],&act_cell)) {
                child->gene[act_cell.r][act_cell.t] = pi->Ac[courses_p1[c]].activities[a];
            }
        }
    }

    // fill courses of p2 to ch1
    for (c = 0; c < n_courses_p2; c++) {
        for (int a = 0; a < pi->Ac[courses_p2[c]].nm_activities; a++) {
            act_in_ind(pi,parent2,pi->Ac[courses_p2[c]].activities[a],&act_cell);
            
            if (strcmp(child->gene[act_cell.r][act_cell.t].id, EmptyActivity.id) == 0) {
                child->gene[act_cell.r][act_cell.t] = pi->Ac[courses_p2[c]].activities[a];
                continue;
            }

            
            size_t act_idx = get_act_idx(pi,pi->Ac[courses_p2[c]].activities[a]);
            int assigned = 0;
            /* assign to different room, same timeslot*/
            for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++){
                if (
                    strcmp(child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t].id, EmptyActivity.id) == 0
                    && is_timeslot_free(pi,child,act_cell.t,courses_p2[c],a)
                ){
                    child->gene[pi->Ra[act_idx].rooms[r].id - 1][act_cell.t] = pi->Ac[courses_p2[c]].activities[a];
                    assigned = 1;
                    break;
                }
            }

            if (assigned) continue;

            /* assign to same rooom, different timeslot */
            for (int t = 0; t < pi->nm_TimeSlots; t++) {
                if (
                    strcmp(child->gene[act_cell.r][t].id, EmptyActivity.id) == 0
                    && is_timeslot_free(pi,child,t,courses_p2[c],a)
                ) {
                    child->gene[act_cell.r][t] = pi->Ac[courses_p2[c]].activities[a];
                    assigned = 1;
                    break;
                }
            }

            if (assigned) continue;
            
            for (int r = 0; r < pi->Ra[act_idx].nm_rooms; r++) {
                for (int t = 0; t < pi->nm_TimeSlots; t++) {
                    if (
                        strcmp(child->gene[pi->Ra[act_idx].rooms[r].id - 1][t].id, EmptyActivity.id) == 0
                        && is_timeslot_free(pi,child,t,courses_p2[c],a)
                    ){
                        child->gene[pi->Ra[act_idx].rooms[r].id - 1][t] = pi->Ac[courses_p2[c]].activities[a];
                        assigned = 1;
                        break;
                    }
                }
                if (assigned) break;
            }
        }
    }

}

void set_satisfied_demand(problem_instance* pi, individual* ind, course_demand *course_enrollments) {
    for (int s = 0; s < pi->nm_Students; s++) {
        for (int c = 0; c < pi->Cs[s].nm_courses; c++) {
            course_enrollments[pi->Cs[s].courses[c].id - 1].sat += ind->student_courses[s][c];
        }
    }
}

int cmpdesc(const void *a, const void *b) {
    return ( ((course_demand *)b)->sat - ((course_demand *)a)->sat );
}

void setup_courses_for_child(problem_instance* pi, course_demand* satisfied_demand_dom, size_t* courses_pdom, size_t* courses_psub, size_t npdom) {
    
    /* sort dominant parent satisfaction in descending order */

    qsort(satisfied_demand_dom,pi->nm_Courses, sizeof(course_demand), cmpdesc);

    int *used_courses_dom = (int *) calloc(pi->nm_Courses,sizeof(int));
    for (int c = 0; c < npdom; c++) {
        courses_pdom[c] = satisfied_demand_dom[c].cid;
        used_courses_dom[satisfied_demand_dom[c].cid] = 1;
    }

    int _idx = 0;
    for (int c = 0; c < pi->nm_Courses; c++) {
        if (used_courses_dom[c]) continue;
        courses_psub[_idx++] = c;
    }

    free(used_courses_dom);
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

        /* sort courses based on accomplished demands */

        course_demand* satisfied_demand_p1 = (course_demand *) calloc(pi->nm_Courses, sizeof(course_demand));
        course_demand* satisfied_demand_p2 = (course_demand *) calloc(pi->nm_Courses, sizeof(course_demand));

        for (int c = 0; c < pi->nm_Courses; c++) {
            satisfied_demand_p1[c] = (course_demand) {
                .cid = c,
                .sat = 0,
            };
            satisfied_demand_p2[c] = (course_demand) {
                .cid = c,
                .sat = 0,
            };
        }

        set_satisfied_demand(pi, parent1, satisfied_demand_p1);
        set_satisfied_demand(pi, parent2, satisfied_demand_p2);

        /* procedure for child 1 */
        setup_courses_for_child(pi, satisfied_demand_p1, courses_p1, courses_p2, n_courses_p1);
        inherit_parents(pi, parent1, parent2, child1, n_courses_p1, n_courses_p2, courses_p1, courses_p2);
        
        /* procedure for child 2 */
        setup_courses_for_child(pi, satisfied_demand_p2, courses_p2, courses_p1, n_courses_p2);
        inherit_parents(pi,parent2,parent1,child2,n_courses_p2,n_courses_p1,courses_p2,courses_p1);

        verify_ind(pi, child1);
        verify_ind(pi, child2);


        free(satisfied_demand_p1);
        free(satisfied_demand_p2);
    }

    else
    {
        child1 = parent1;
        child2 = parent2;
    }
}