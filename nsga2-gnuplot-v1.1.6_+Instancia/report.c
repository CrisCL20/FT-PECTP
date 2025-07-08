/* Routines for storing population data into files */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/* Function to print the information of a population in a file */
void report_pop(population *pop, FILE *fpt)
{
    int i, j, k;
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

/* Function to print the information of feasible and non-dominated population in a file */
void report_feasible(population *pop, FILE *fpt, double elapsed)
{
    int i, j, k;
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
    return;
}
