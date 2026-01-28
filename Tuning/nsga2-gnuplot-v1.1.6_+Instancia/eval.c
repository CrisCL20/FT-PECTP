/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "rand.h"

int maxprint = 1;
/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        evaluate_ind(&(pop->ind[i]), pi);
    }
    return;
}

/* Routine to evaluate objective function values and constraints for an individual */
/*void evaluate_ind (individual *ind)
{
    int j;
    test_problem (ind->xreal, ind->xbin, ind->gene, ind->obj, ind->constr);
    if (ncon==0)
    {
        ind->constr_violation = 0.0;
    }
    else
    {
        ind->constr_violation = 0.0;
        for (j=0; j<ncon; j++)
        {
            if (ind->constr[j]<0.0)
            {
                ind->constr_violation += ind->constr[j];
            }
        }
    }
    return;
}*/

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind, problem_instance *pi)
{
    /*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

    int j;
    coveringIndividual(ind, pi,0);
    preferenceIndividual(ind, pi,0);

    // restrictions
    if (ncon == 0)
    {
        ind->constr_violation = 0.0;
    }
    else
    {

        ind->constr_violation = 0.0;
        int horizon_length = pi->horizon_length;
        int num_employees = pi->num_employees;
        int num_shifts = pi->num_shifts;

        for (int i = 0; i < num_employees; i++)
        {
            int consecutive_shifts = 0;
            int consecutive_off = 0;
            int total_minutes = 0;
            int weekend_count = 0;
            int *shifts_count = calloc(num_shifts, sizeof(int));

            for (int d = 0; d < horizon_length; d++)
            {
                int shift = ind->xreal[d * num_employees + i];

                // Shift rotation
                if (d > 0)
                {
                    int prev_shift = ind->xreal[(d - 1) * num_employees + i];
                    if (is_incompatible(shift, pi->shifts[prev_shift].incompatible_shifts, pi->shifts[prev_shift].num_incompatible_shifts))
                    {
                        if(maxprint){
                            printf("Incompatible shift: employee %d, day %d, shift %d\n", i, d, shift);
                        }
                        ind->constr_violation -= 1;
                    }
                }
                // Min consecutive shifts and days off
                // if(maxprint){
                //     printf("Employee: %s, consecutive_shifts: %d, consecutive_off: %d\n", pi->employees[i].name, consecutive_shifts, consecutive_off);
                // }
                if ((shift == 0 && consecutive_shifts > 0 && consecutive_shifts < pi->employees[i].min_consecutive_shifts) ||
                    (shift != 0 && consecutive_off > 0 && consecutive_off < pi->employees[i].min_consecutive_days_off))
                {
                    if (maxprint){
                        printf("Consecutive off: employee %d: %d\n", i, consecutive_off);
                    }
                    ind->constr_violation -= 1;
                }

                // Count shifts and minutes
                if (shift != 0)
                {
                    shifts_count[shift]++;
                    total_minutes += pi->shifts[shift].length;
                    consecutive_shifts++;
                    consecutive_off = 0;
                }
                else
                {
                    consecutive_off++;
                    consecutive_shifts = 0;
                }

                // Max consecutive shifts
                if (consecutive_shifts > pi->employees[i].max_consecutive_shifts)
                {
                    if(maxprint){
                        printf("Consecutive shifts: employee %d: %d\n", i, consecutive_shifts);
                    }

                    ind->constr_violation -= 1;
                }

                

                // Weekend work
                if ((d % 7 == 5 ))
                {
                    //check next day
                    if (shift != 0)
                    {
                        weekend_count++;
                    }else{
                        if (d + 1 < horizon_length)
                        {
                        int next_shift = ind->xreal[(d + 1) * num_employees + i];
                        
                        if (next_shift != 0){
                            weekend_count++;
                            }
                        }
                    }
                    
                    
                }
            }

            // Max shifts of each type
            for (int j = 0; j < num_shifts; j++)
            {
                if (shifts_count[j] > pi->employees[i].max_shifts[j])
                {
                    if(maxprint){
                        printf("Shift count: employee %d, shift %d: %d\n", i, j, shifts_count[j]);
                    }
                    ind->constr_violation -= 1;
                }
            }

            // Min and max work time
            if (total_minutes < pi->employees[i].min_total_minutes || total_minutes > pi->employees[i].max_total_minutes)
            {
                
                //ponderate the violation by the difference
                if (total_minutes < pi->employees[i].min_total_minutes){
                    ind->constr_violation -= (pi->employees[i].min_total_minutes - total_minutes)/100;
                    if(maxprint){
                        printf("Total minutes: employee %s, penalty: %d\n", pi->employees[i].name, (pi->employees[i].min_total_minutes - total_minutes)/100);
                    }
                }else{
                    ind->constr_violation -= (total_minutes - pi->employees[i].max_total_minutes)/100;
                    if(maxprint){
                        printf("Total minutes: employee %s, penalty: %d\n", pi->employees[i].name, (total_minutes - pi->employees[i].max_total_minutes)/100);
                    }
                }   

                
            }

            // Max working weekends
            if (weekend_count > pi->employees[i].max_weekends)
            {
                if(maxprint){
                    printf("Week count: employee %s: %d\n", pi->employees[i].name, weekend_count);
                }
                ind->constr_violation -= 1;
            }

            free(shifts_count);
            
        }
        maxprint = 0;
    }

    return;
}
