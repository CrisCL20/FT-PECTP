/* Mutation routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

/* Function to perform mutation in a population */
void mutation_pop (population *pop, problem_instance *pi)
{
    int i;
    for (i=0; i<popsize; i++)
    {
        double rand = randomperc();
        // mutation_ind(&(pop->ind[i]));  //best prob is 0.02
        // mutation_ind_2exchange(&(pop->ind[i]), pi);
        mutation_ind_block_exchange(&(pop->ind[i]), 3, pi); 
        // mutation_ind_swap(&(pop->ind[i]), pi); //
        // mutation_ind_block_swap(&(pop->ind[i]), 3, pi);  //best prob is 0.1
        // if(rand <= 0.2){
        //     mutation_ind(&(pop->ind[i]));
        // }else if(rand <= 0.4){
        //     mutation_ind_2exchange(&(pop->ind[i]), pi);
        // }else if(rand <= 0.6){
        //     mutation_ind_block_exchange(&(pop->ind[i]), 3, pi);
        // }else if(rand <= 0.8){
        //     mutation_ind_swap(&(pop->ind[i]), pi);
        // }else{
        //     mutation_ind_block_swap(&(pop->ind[i]), 3, pi);

        // }
    }

    //2-exc
    return;
}

/* Function to perform mutation of an individual */
void mutation_ind (individual *ind)
{
    if (nreal!=0)
    {
        real_mutate_ind(ind);
    }
    if (nbin!=0)
    {
        bin_mutate_ind(ind);
    }
    return;
}

/* Routine for binary mutation of an individual */
void bin_mutate_ind (individual *ind)
{
    int j, k;
    double prob;
    for (j=0; j<nbin; j++)
    {
        for (k=0; k<nbits[j]; k++)
        {
            prob = randomperc();
            if (prob <=pmut_bin)
            {
                if (ind->gene[j][k] == 0)
                {
                    ind->gene[j][k] = 1;
                }
                else
                {
                    ind->gene[j][k] = 0;
                }
                nbinmut+=1;
            }
        }
    }
    return;
}

/* Routine for real polynomial mutation of an individual */
void real_mutate_ind (individual *ind)
{
    int j;
    double rnd, delta1, delta2, mut_pow, deltaq;
    int y, yl, yu, val, xy;
    for (j = 0; j < nreal; j++)
    {
        if (randomperc() <= pmut_real)
        {
            y = ind->xreal[j];
            yl = min_realvar[j];
            yu = max_realvar[j];

            if (yu == yl) // Evitar división por cero
            {
                continue; // Saltar la mutación si el rango es cero
            }

            delta1 = (y - yl) / (yu - yl);
            delta2 = (yu - y) / (yu - yl);
            rnd = randomperc();
            mut_pow = 1.0 / (eta_m + 1.0);
            
            if (rnd <= 0.5)
            {
                xy = 1 - delta1;
                val = 2 * rnd + (1 - 2 * rnd) * pow(xy, (eta_m + 1.0));

                if (val < 0) // Evitar cálculo indefinido
                {
                    val = 0;
                }

                deltaq = pow(val, mut_pow) - 1;
            }
            else
            {
                xy = 1 - delta2;
                val = 2 * (1 - rnd) + 2 * (rnd - 0.5) * pow(xy, (eta_m + 1.0));
                
                if (val < 0) // Evitar cálculo indefinido
                {
                    val = 0;
                }

                deltaq = 1 - pow(val, mut_pow);
            }

            y = y + deltaq * (yu - yl); // Eliminar el cast a entero
            if (y < yl)
                y = yl;
            if (y > yu)
                y = yu;
            
            ind->xreal[j] = y;
            nrealmut += 1;
        }
    }
    return;
}


//mutation 2-exchange

void mutation_ind_2exchange (individual *ind, problem_instance *pi){
    double prob = randomperc();
    if (prob <= pmut_real){
        //exchange shift from 2 employees
        for (int i = 0; i < pi->num_employees; i++)
        {
            if(randomperc() <= 0.5){
                int employee2=rnd(0, pi->num_employees);
                while (i==employee2){
                    employee2=rnd(0, pi->num_employees);
                }
                int day = rnd(0, pi->horizon_length);
                int shift1 = ind->xreal[day * pi->num_employees + i];
                int shift2 = ind->xreal[day * pi->num_employees + employee2];
                if (shift1 != -1 && shift2 != -1){
                    ind->xreal[day * pi->num_employees + i] = shift2;
                    ind->xreal[day * pi->num_employees + employee2] = shift1;
                    
                }
            }
        }
        
       
        
    

    }
}

void mutation_ind_block_exchange (individual *ind,int block_size, problem_instance *pi){
    double prob = randomperc();
    if (prob <= pmut_real){
        //exchange shift from 2 employees
        int employee1=rnd(0, pi->num_employees);
        int employee2=rnd(0, pi->num_employees);
        while (employee1==employee2){
            employee2=rnd(0, pi->num_employees);
        }
        int day = rnd(0, pi->horizon_length-block_size);
        for (int i = 0; i < block_size; i++){
            int shift1 = ind->xreal[(day+i) * pi->num_employees + employee1];
            int shift2 = ind->xreal[(day+i) * pi->num_employees + employee2];
            if (shift1 != -1 && shift2 != -1){
                ind->xreal[(day+i) * pi->num_employees + employee1] = shift2;
                ind->xreal[(day+i) * pi->num_employees + employee2] = shift1;
            }
        }

    }
}

void mutation_ind_swap (individual *ind, problem_instance *pi){
    double prob = randomperc();
    if (prob <= pmut_real){
        for(int i=0; i<pi->num_employees; i++){
            if(randomperc() <= 0.5){
                int day1 = rnd(0, pi->horizon_length);
                int day2 = rnd(0, pi->horizon_length);
                while(day1 == day2){
                    day2 = rnd(0, pi->horizon_length);
                }
                int shift1 = ind->xreal[day1 * pi->num_employees + i];
                int shift2 = ind->xreal[day2 * pi->num_employees + i];
                ind->xreal[day1 * pi->num_employees + i] = shift2;
                ind->xreal[day2 * pi->num_employees + i] = shift1;
            }
        }
    }
}

void mutation_ind_block_swap (individual *ind, int size, problem_instance*pi){
    double prob = randomperc();
    if(prob <= pmut_real && size <= pi->horizon_length/2){
        for (int i=0; i<pi->num_employees; i++){
            if(randomperc()<=0.5){
                int day1 = rnd(0, pi->horizon_length-size);
                int day2 = rnd(0, pi->horizon_length-size);
                while(day1-day2<size && day1-day2>-size){
                    day2 = rnd(0, pi->horizon_length-size);
                }
                for(int j=0; j<size; j++){
                    int shift1 = ind->xreal[(day1+j) * pi->num_employees + i];
                    int shift2 = ind->xreal[(day2+j) * pi->num_employees + i];
                    ind->xreal[(day1+j) * pi->num_employees + i] = shift2;
                    ind->xreal[(day2+j) * pi->num_employees + i] = shift1;
                }
            }
        }
    }


}