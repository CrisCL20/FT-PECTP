/* Crossover routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"

/* Function to cross two individuals */
void crossover (individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{
    if (nreal!=0)
    {
        //realcross (parent1, parent2, child1, child2);
        //cross_employee(parent1, parent2, child1, child2, pi);
        cross_days(parent1, parent2, child1, child2, pi);

    }
    if (nbin!=0)
    {
        bincross (parent1, parent2, child1, child2);
    }
    return;
}

/* Routine for real variable SBX crossover */
void realcross (individual *parent1, individual *parent2, individual *child1, individual *child2)
{
    int i;
    double rand;
    int y1, y2, yl, yu;
    int c1, c2;
    double alpha, beta, betaq;
    if (randomperc() <= pcross_real)
    {
        nrealcross++;
        for (i=0; i<nreal; i++)
        {
            if (randomperc()<=0.5 )
            {
                if (fabs(parent1->xreal[i]-parent2->xreal[i]) > EPS)
                {
                    if (parent1->xreal[i] < parent2->xreal[i])
                    {
                        y1 = parent1->xreal[i];
                        y2 = parent2->xreal[i];
                    }
                    else
                    {
                        y1 = parent2->xreal[i];
                        y2 = parent1->xreal[i];
                    }
                    yl = min_realvar[i];
                    yu = max_realvar[i];
                    rand = randomperc();
                    beta = 1.0 + (2.0*(y1-yl)/(y2-y1));
                    alpha = 2.0 - pow(beta,-(eta_c+1.0));
                    if (rand <= (1.0/alpha))
                    {
                        betaq = pow ((rand*alpha),(1.0/(eta_c+1.0)));
                    }
                    else
                    {
                        betaq = pow ((1.0/(2.0 - rand*alpha)),(1.0/(eta_c+1.0)));
                    }
                    c1 = 0.5*((y1+y2)-betaq*(y2-y1));
                    beta = 1.0 + (2.0*(yu-y2)/(y2-y1));
                    alpha = 2.0 - pow(beta,-(eta_c+1.0));
                    if (rand <= (1.0/alpha))
                    {
                        betaq = pow ((rand*alpha),(1.0/(eta_c+1.0)));
                    }
                    else
                    {
                        betaq = pow ((1.0/(2.0 - rand*alpha)),(1.0/(eta_c+1.0)));
                    }
                    c2 = (int)(0.5 * ((y1 + y2) + betaq * (y2 - y1)));
                    if (c1<yl)
                        c1=yl;
                    if (c2<yl)
                        c2=yl;
                    if (c1>yu)
                        c1=yu;
                    if (c2>yu)
                        c2=yu;
                    if (randomperc()<=0.5)
                    {
                        child1->xreal[i] = c2;
                        child2->xreal[i] = c1;
                    }
                    else
                    {
                        child1->xreal[i] = c1;
                        child2->xreal[i] = c2;
                    }
                }
                else
                {
                    child1->xreal[i] = parent1->xreal[i];
                    child2->xreal[i] = parent2->xreal[i];
                }
            }
            else
            {
                child1->xreal[i] = parent1->xreal[i];
                child2->xreal[i] = parent2->xreal[i];
            }
        }
    }
    else
    {
        for (i=0; i<nreal; i++)
        {
            child1->xreal[i] = parent1->xreal[i];
            child2->xreal[i] = parent2->xreal[i];
        }
    }
    return;
}

/* Routine for two point binary crossover */
void bincross (individual *parent1, individual *parent2, individual *child1, individual *child2)
{
    int i, j;
    double rand;
    int temp, site1, site2;
    for (i=0; i<nbin; i++)
    {
        rand = randomperc();
        if (rand <= pcross_bin)
        {
            nbincross++;
            site1 = rnd(0,nbits[i]-1);
            site2 = rnd(0,nbits[i]-1);
            if (site1 > site2)
            {
                temp = site1;
                site1 = site2;
                site2 = temp;
            }
            for (j=0; j<site1; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
            for (j=site1; j<site2; j++)
            {
                child1->gene[i][j] = parent2->gene[i][j];
                child2->gene[i][j] = parent1->gene[i][j];
            }
            for (j=site2; j<nbits[i]; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
        }
        else
        {
            for (j=0; j<nbits[i]; j++)
            {
                child1->gene[i][j] = parent1->gene[i][j];
                child2->gene[i][j] = parent2->gene[i][j];
            }
        }
    }
    return;
}



void cross_employee(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{
    double rand = randomperc();
    if (rand <= pcross_real)
    {
        //for each employee
        
        
        for (int i = 0; i < pi->num_employees; i++)
        {
            //one employee to child1 and the other to child2
            if (randomperc() <= 0.5){
                for (int j = 0; j < pi->horizon_length; j++)
                {
                    child1->xreal[j * pi->num_employees + i] = parent1->xreal[j * pi->num_employees + i];
                    child2->xreal[j * pi->num_employees + i] = parent2->xreal[j * pi->num_employees + i];
                }
            }
            else
            {
                for (int j = 0; j < pi->horizon_length; j++)
                {
                    child1->xreal[j * pi->num_employees + i] = parent2->xreal[j * pi->num_employees + i];
                    child2->xreal[j * pi->num_employees + i] = parent1->xreal[j * pi->num_employees + i];
                }
            }
        }
    }
    else{
        for (int i = 0; i < nreal; i++)
        {
            child1->xreal[i] = parent1->xreal[i];
            child2->xreal[i] = parent2->xreal[i];
        }
    }
}

void cross_days(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi)
{
    double rand = randomperc();
    if (rand <= pcross_real)
    {
        //for each day
        int rand_day = rnd(0, pi->horizon_length-1);
        for (int i = 0; i < pi->horizon_length; i++)
        {
            if (i<=rand_day){
                for (int j = 0; j < pi->num_employees; j++)
                {
                    child1->xreal[i * pi->num_employees + j] = parent1->xreal[i * pi->num_employees + j];
                    child2->xreal[i * pi->num_employees + j] = parent2->xreal[i * pi->num_employees + j];
                }
            }
            else{
                for (int j = 0; j < pi->num_employees; j++)
                {
                    child1->xreal[i * pi->num_employees + j] = parent2->xreal[i * pi->num_employees + j];
                    child2->xreal[i * pi->num_employees + j] = parent1->xreal[i * pi->num_employees + j];
                }
            }
        }
    }
    else{
        for (int i = 0; i < nreal; i++)
        {
            child1->xreal[i] = parent1->xreal[i];
            child2->xreal[i] = parent2->xreal[i];
        }
    }
}