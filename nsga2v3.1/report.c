/* Routines for storing population data into files */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

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

/* Function to print the information of feasible and non-dominated population in a file */
void report_feasible(problem_instance *pi, population *pop, size_t popsize, FILE *fpt, double elapsed)
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

    // write the solution closest to (0,0) to csv file

    qsort(pop->ind, popsize, sizeof(individual), compar);

    individual *best_ind = &(pop->ind[0]);

    FILE *studentsFile = fopen("../studentsFile.csv", "w");
    FILE *coursesFile = fopen("../coursesFile.csv", "w");

    fprintf(studentsFile, "id_student;id_course\n");
    fprintf(coursesFile, "id_course;id_activity;id_room;id_timeslot\n");

    for (i = 0; i < pi->nm_Students; i++)
        for (j = 0; j < pi->Cs[i].nm_courses; j++)
            if (best_ind->student_courses[i][j])
                fprintf(studentsFile, "%d;%d\n", pi->S[i].id, pi->Cs[i].courses[j].id);

    for (i = 0; i < pi->nm_Courses; i++)
    {
        for (j = 0; j < pi->Ac[i].nm_activities; j++)
        {
            t_cellTuple *act_cell = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
            size_t act_idx = get_act_idx(pi, pi->Ac[i].activities[j]);
            act_in_ind(pi, best_ind, pi->A[act_idx], act_cell);

            if (act_cell != NULL)
                fprintf(coursesFile, "%d;%s;%d;%s\n", pi->C[i].id, pi->Ac[i].activities[j].id, pi->R[act_cell->r].id, pi->T[act_cell->t].ts);

            free(act_cell);
        }
    }

    fflush(studentsFile);
    fflush(coursesFile);
    fclose(studentsFile);
    fclose(coursesFile);

    return;
}
