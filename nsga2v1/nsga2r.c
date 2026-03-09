/* NSGA-II routine (implementation of the 'main' function) */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#include "global.h"
#include "rand.h"

int nreal;
int nbin;
int nobj;
int ncon;
int popsize;
double pcross_real;
double pcross_bin;
double pmut_real;
double pmut_bin;
double eta_c;
double eta_m;
int ngen;
int nbinmut;
int nrealmut;
int nbincross;
int nrealcross;
int *nbits;
double *min_realvar;
double *max_realvar;
double *min_binvar;
double *max_binvar;
int bitlength;
int choice;
int obj1;
int obj2;
int obj3;
int angle1;
int angle2;

int main(int argc, char **argv)
{
    int i;
    FILE *fpt1;
    FILE *fpt2;
    FILE *fpt3;
    FILE *fpt4;
    FILE *fpt5;

    problem_instance *pi = malloc(sizeof(problem_instance));
    population *parent_pop;
    population *child_pop;
    population *mixed_pop;

    if (argc < 2)
    {
        printf("\n Usage ./nsga2r instance_route random_seed popsize ngen nobj pcross_bin pmut_bin\n./nsga2r 0.123 b-Instancia14_cap2_relacion7UnoUnoUnoTodosDistintos.dat 100 100 2 0.6 0.01\n");
        exit(1);
    }
    seed = (double)atof(argv[1]);
    if (seed <= 0.0 || seed >= 1.0)
    {
        printf("\n Entered seed value is wrong, seed value must be in (0,1) \n");
        exit(1);
    }
    fpt1 = fopen("initial_pop.out", "w");
    fpt2 = fopen("final_pop.out", "w");
    fpt3 = fopen("best_pop.out", "w");
    fpt4 = fopen("all_pop.out", "w");
    fpt5 = fopen("params.out", "w");
    fprintf(fpt1, "# This file contains the data of initial population\n");
    fprintf(fpt2, "# This file contains the data of final population\n");
    fprintf(fpt3, "# This file contains the data of final feasible population (if found)\n");
    fprintf(fpt4, "# This file contains the data of all generations\n");
    fprintf(fpt5, "# This file contains information about inputs as read by the program\n");

    char *instance_route = argv[2];
    printf("Reading instance file %s...\n", instance_route);
    readInputFile(instance_route, pi);
    printf("DONE\n");

    // printProblemInstance(pi);

    popsize = atoi(argv[3]);
    if (popsize < 4 || (popsize % 4) != 0)
    {
        printf("\n population size read is : %d", popsize);
        printf("\n Wrong population size entered, hence exiting \n");
        exit(1);
    }
    ngen = atoi(argv[4]);
    if (ngen < 1)
    {
        printf("\n number of generations read is : %d", ngen);
        printf("\n Wrong nuber of generations entered, hence exiting \n");
        exit(1);
    }
    nobj = atoi(argv[5]);
    if (nobj < 1)
    {
        printf("\n number of objectives entered is : %d", nobj);
        printf("\n Wrong number of objectives entered, hence exiting \n");
        exit(1);
    }

    pcross_bin = atof(argv[6]);
    if (pcross_bin < 0.0 || pcross_bin > 1.0)
    {
        printf("\n Probability of crossover entered is : %e", pcross_bin);
        printf("\n Entered value of probability of crossover of binary variables is out of bounds, hence exiting \n");
        exit(1);
    }
    pmut_bin = atof(argv[7]);
    if (pmut_bin < 0.0 || pmut_bin > 1.0)
    {
        printf("\n Probability of mutation entered is : %e", pmut_bin);
        printf("\n Entered value of probability  of mutation of binary variables is out of bounds, hence exiting \n");
        exit(1);
    }

    printf("\n Input data successfully entered, now performing initialization \n");
    fprintf(fpt5, "\n Population size = %d", popsize);
    fprintf(fpt5, "\n Number of generations = %d", ngen);
    fprintf(fpt5, "\n Number of objective functions = %d", nobj);

    fprintf(fpt5, "\n Number of binary variables = %d", nbin);
    if (nbin != 0)
    {
        for (i = 0; i < nbin; i++)
        {
            fprintf(fpt5, "\n Number of bits for binary variable %d = %d", i + 1, nbits[i]);
            fprintf(fpt5, "\n Lower limit of binary variable %d = %e", i + 1, min_binvar[i]);
            fprintf(fpt5, "\n Upper limit of binary variable %d = %e", i + 1, max_binvar[i]);
        }
        fprintf(fpt5, "\n Probability of crossover of binary variable = %e", pcross_bin);
        fprintf(fpt5, "\n Probability of mutation of binary variable = %e", pmut_bin);
    }
    fprintf(fpt5, "\n Seed for random number generator = %e", seed);
    bitlength = 0;
    if (nbin != 0)
    {
        for (i = 0; i < nbin; i++)
        {
            bitlength += nbits[i];
        }
    }
    fprintf(fpt1, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
    fprintf(fpt2, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
    fprintf(fpt3, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
    fprintf(fpt4, "# of objectives = %d, # of constraints = %d, # of real_var = %d, # of bits of bin_var = %d, constr_violation, rank, crowding_distance\n", nobj, ncon, nreal, bitlength);
    clock_t tic = clock();
    nbinmut = 0;
    nrealmut = 0;
    nbincross = 0;
    nrealcross = 0;
    parent_pop = (population *)malloc(sizeof(population));
    child_pop = (population *)malloc(sizeof(population));
    mixed_pop = (population *)malloc(sizeof(population));
    printf("\n Allocating memory for population...");
    allocate_memory_pop(parent_pop, popsize, pi);
    allocate_memory_pop(child_pop, popsize, pi);
    allocate_memory_pop(mixed_pop, 2 * popsize, pi);
    randomize();
    printf("\n Initializing population...");
    initialize_pop(parent_pop, pi);
    printf("\n Initialization done, now performing first generation\n");

    printf("Evaluating pop\n");
    evaluate_pop(parent_pop, pi);
    printf("Evaluated pop\n");
    assign_rank_and_crowding_distance(parent_pop);
    report_pop(parent_pop, fpt1);
    fprintf(fpt4, "# gen = 1\n");
    report_pop(parent_pop, fpt4);
    // printf("\n gen = 1");
    fflush(stdout);
    /*if (choice!=0)
        onthefly_display (parent_pop,gp,1);*/
    fflush(fpt1);
    fflush(fpt2);
    fflush(fpt3);
    fflush(fpt4);
    fflush(fpt5);
    sleep(1);
    for (i = 2; i <= ngen; i++)
    {
        selection(parent_pop, child_pop, pi);
        mutation_pop(child_pop, pi);
        evaluate_pop(child_pop, pi);
        merge(parent_pop, child_pop, mixed_pop);
        fill_nondominated_sort(mixed_pop, parent_pop);

        /* Comment following four lines if information for all
        generations is not desired, it will speed up the execution */

        // fprintf(fpt4, "# gen = %d\n", i);
        // report_pop(parent_pop, fpt4);
        // fflush(fpt4);
        printf("\n gen = %d", i);
    }
    clock_t toc = clock();
    double elapsed = (double)(toc - tic) / CLOCKS_PER_SEC;
    printf("\n Generations finished, now reporting solutions");
    report_pop(parent_pop, fpt2);
    report_feasible(parent_pop, fpt3, elapsed);
    if (nreal != 0)
    {
        fprintf(fpt5, "\n Number of crossover of real variable = %d", nrealcross);
        fprintf(fpt5, "\n Number of mutation of real variable = %d", nrealmut);
    }
    if (nbin != 0)
    {
        fprintf(fpt5, "\n Number of crossover of binary variable = %d", nbincross);
        fprintf(fpt5, "\n Number of mutation of binary variable = %d", nbinmut);
    }
    fflush(stdout);
    fflush(fpt1);
    fflush(fpt2);
    fflush(fpt3);
    fflush(fpt4);
    fflush(fpt5);
    fclose(fpt1);
    fclose(fpt2);
    fclose(fpt3);
    fclose(fpt4);
    fclose(fpt5);
    if (nbin != 0)
    {
        free(min_binvar);
        free(max_binvar);
        free(nbits);
    }
    deallocate_memory_pop(parent_pop, popsize, pi);
    deallocate_memory_pop(child_pop, popsize, pi);
    deallocate_memory_pop(mixed_pop, 2 * popsize, pi);
    free(parent_pop);
    free(child_pop);
    free(mixed_pop);
    printf("\n Routine successfully exited \n");
    return (0);
}
