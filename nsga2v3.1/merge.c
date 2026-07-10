/* Routine for mergeing two populations */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "rand.h"

/* Routine to merge two populations into one */
void merge(population *pop1, population *pop2, population *pop3)
{
    int i, k;
    for (i = 0; i < popsize; i++)
    {
        copy_ind(&(pop1->ind[i]), &(pop3->ind[i]));
    }
    for (i = 0, k = popsize; i < popsize; i++, k++)
    {
        copy_ind(&(pop2->ind[i]), &(pop3->ind[k]));
    }
    return;
}

/* Routine to copy an individual 'ind1' into another individual 'ind2' */
void copy_ind(individual *ind1, individual *ind2)
{
    int i;
    ind2->rank = ind1->rank;
    ind2->constr_violation = ind1->constr_violation;
    ind2->crowd_dist = ind1->crowd_dist;

    for (i = 0; i < nobj; i++)
    {
        ind2->obj[i] = ind1->obj[i];
    }
    
    for (int r = 0; r < g_pi->nm_Rooms; r++)
        for (int t = 0; t < g_pi->nm_TimeSlots; t++)
            ind2->gene[r][t] = ind1->gene[r][t];
    
    for (int s = 0; s < g_pi->nm_Students; s++)
        for (int c = 0; c < g_pi->Cs[s].nm_courses; c++)
            ind2->student_courses[s][c] = ind1->student_courses[s][c];
    
    /*
    if (ncon!=0)
    {
        for (i=0; i<ncon; i++)
        {
            ind2->constr[i] = ind1->constr[i];
        }
    }
    */
    return;
}
