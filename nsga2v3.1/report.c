/* Routines for storing population data into files */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

const int REPORT_SOLUTIONS = 0; 

/* Function to print the information of a population in a file */
void report_pop(population *pop, FILE *fpt)
{
    int i, j;
    for (i = 0; i < popsize; i++)
    {
        for (j = 0; j < nobj; j++)
        {
            fprintf(fpt, "%e\t", pop->ind[i].obj[j]);
        }

        fprintf(fpt, "%e\t", pop->ind[i].constr_violation);
        fprintf(fpt, "%d\t", pop->ind[i].rank);
        fprintf(fpt, "%e\n", pop->ind[i].crowd_dist);
    }
    return;
}

int compar(const void *a, const void *b)
{
    const individual *indA = (const individual *)a;
    const individual *indB = (const individual *)b;

    size_t distA = indA->obj[0] + indA->obj[1];
    size_t distB = indB->obj[0] + indB->obj[1];

    if (distA < distB)
        return -1;
    if (distA > distB)
        return 1;
    return 0;
}

void report_solutions(problem_instance* pi, population* pop, size_t popsize, char* instance_name) {
    int i;
    
    for(i = 0; i < popsize; i++) {
        if (!(pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1)) continue;
        
        float obj_1 = (&(pop->ind[i]))->obj[0];
        float obj_2 = (&(pop->ind[i]))->obj[1];
        
        char student_filename[256];
        int cx = snprintf(student_filename, sizeof(student_filename), "studentsFile_%s_s%.2f_o1%f_o2%f.csv", instance_name, seed, obj_1, obj_2);
        if (cx < 0) {
          fprintf(stderr, "Could not format final pop output file.\n");
          exit(EXIT_FAILURE);
        }
        char courses_filename[256];
        cx = snprintf(courses_filename, sizeof(courses_filename), "coursesFile_%s_s%.2f_o1%f_o2%f.csv", instance_name, seed, obj_1, obj_2);
        if (cx < 0) {
          fprintf(stderr, "Could not format final pop output file.\n");
          exit(EXIT_FAILURE);
        }
        FILE *studentsFile = fopen(student_filename, "w");
        FILE *coursesFile = fopen(courses_filename, "w");
        
        fprintf(studentsFile, "id_student;id_course\n");
        fprintf(coursesFile, "id_course;id_activity;id_room;id_timeslot\n");
        int s,c;
        // for (s = 0; s < pi->nm_Students; s++)
        //     for (c = 0; c < pi->Cs[s].nm_courses; c++)
        //         if ((&(pop->ind[i]))->student_courses[s][c])
        //             fprintf(studentsFile, "%d;%d\n", pi->S[s].id, pi->Cs[s].courses[c].id);
        
        int a;
        for (c = 0; c < pi->nm_Courses; c++)
        {
            for (a = 0; a < pi->Ac[c].nm_activities; a++)
            {
                t_cellTuple cell;
                size_t act_idx = pi->Ac[c].activity_idx[a];
                act_in_ind(pi, &(pop->ind[i]), pi->A[act_idx], &cell);
                fprintf(coursesFile, "%d;%s;%d;%s\n", pi->C[c].id, pi->Ac[c].activities[a].id, pi->R[cell.r].id, pi->T[cell.t].ts);
    
            }
        }   
        fflush(studentsFile);
        fflush(coursesFile);
        fclose(studentsFile);
        fclose(coursesFile);
    }
}

/* Function to print the information of feasible and non-dominated population in a file */
void report_feasible(problem_instance *pi, population *pop, size_t popsize, FILE *fpt, double elapsed, char* instance_name)
{
    int i, j;
    for (i = 0; i < popsize; i++)
    {
        if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1)
        {
            for (j = 0; j < nobj; j++)
            {
                fprintf(fpt, "%e\t", pop->ind[i].obj[j]);
            }
            fprintf(fpt, "%e\t", pop->ind[i].constr_violation);
            fprintf(fpt, "%d\t", pop->ind[i].rank);
            fprintf(fpt, "%e\n", pop->ind[i].crowd_dist);
        }
    }
    fprintf(fpt, "Total execution time: %.3lf\n", elapsed);
    
    if (REPORT_SOLUTIONS)
        report_solutions(pi, pop, popsize, instance_name);

    return;
}

void report_objectives(size_t gen, population* pop, size_t popsize, FILE* fpt){
    int i, j;
    for (i = 0; i < popsize; i++)
    {
        if (pop->ind[i].constr_violation == 0.0 && pop->ind[i].rank == 1)
        {
            fprintf(fpt, "%ld;", gen);
            for (j = 0; j < nobj; j++)
            {
                fprintf(fpt, "%e;", pop->ind[i].obj[j]);
            }
            fprintf(fpt, "\n");
        }
    }
}

void verify_pop(population *pop, problem_instance* pi) {
    for (int i = 0; i < popsize; i++) {
        verify_ind(pi, &pop->ind[i]);
    }
}