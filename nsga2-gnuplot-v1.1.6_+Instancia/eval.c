/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop(population *pop)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        evaluate_ind(&(pop->ind[i]));
    }
    return;
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

int attends_class(unsigned sid, unsigned cid, room_ts_pair *gene, problem_instance *pi)
{
    char class_tslot[10] = gene[cid].ts;

    // un estudiante atiende a una clase si ésta es parte de un módulo que le interesa
    unsigned nmodprefs = pi->mod_prefs[sid].nmods;
    module *modprefs = pi->mod_prefs[sid].mods;

    int i;
    for (i = 0; i < nmodprefs; i++)
    {
    }

    return 1;
}

/*FO1: preferencias horarias*/
unsigned long countTimesRequestsMet(room_ts_pair *gene, problem_instance *pi)
{
    int i, j;
    unsigned long c;

    for (i = 0; i < pi->S; i++)
    {
        unsigned nprefs = pi->tslot_prefs[i].nprefs;
        tslot *prefs = pi->tslot_prefs[i].tslots;

        for (j = 0; j < pi->C; j++)
        {
            if (/*estudiante i asiste a la clase j && not (gen[j].ts isin(prefs))*/)
            /*c++*/
        }
    }
}

/*FO2: cantidad de modulos preferidos no asignados*/
unsigned long countModRequestsMet(room_ts_pair *gene, problem_instance *pi)
{
    int i, j;

    for (i = 0; i < pi->S; i++)
    {
    }
}

void test_problem(room_ts_pair *gene, double *obj, double *constr, problem_instance *pi)
{

    return;
}

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind)
{

    int j;
    /*test_problem (ind->xreal, ind->xbin, ind->gene, ind->obj, ind->constr);*/
    if (ncon == 0)
    {
        ind->constr_violation = 0.0;
    }
    else
    {
        ind->constr_violation = 0.0;
        for (j = 0; j < ncon; j++)
        {
            if (ind->constr[j] < 0.0)
            {
                ind->constr_violation += ind->constr[j];
            }
        }
    }
    return;
}
