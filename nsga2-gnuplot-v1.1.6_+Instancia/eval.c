/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

/* Routine to evaluate objective function values and constraints for a population */
void evaluate_pop(population *pop, const problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        evaluate_ind(&(pop->ind[i]), pi);
    }
    return;
}

/*Acá la evaluación completa. Deben setearse los valores de obj y constr_violation. */

int get_ts_block_idx(const unsigned b1, const unsigned T)
{
    return ((2 * T / 5) - b1 - 1) / 2;
}

/*FO1: preferencias horarias*/
void countTimesRequestsMet(unsigned **student_schedule, unsigned **gene, double *obj, problem_instance *pi)
{
    int i, j, k, r;
    unsigned long counts = 0;

    for (i = 0; i < pi->S; i++)
    {
        /*count where the student goes to classes in a time period they did not request*/
        unsigned n_prefs = pi->tslot_prefs[i].nprefs;
        char ts[10];
        for (j = 0; j < n_prefs; j++)
        {
            strcpy(ts, pi->tslot_prefs[i].tslots[j].ts);
            char **tokens = str_split(ts, '_');
            unsigned ts_idx; /*idx of the student preference*/

            if (tokens)
            {
                unsigned d = atol(tokens[0]);
                unsigned b1 = atol(tokens[1]);

                ts_idx = ((pi->T / 5) * d) - 1 - get_ts_block_idx(b1, pi->T);
            }

            free(tokens);

            /*if the student does not attend to a class in that timeslot, increase counts*/
            /*find classes the student takes -> see if one of those classes is scheduled in ts_idx*/
            /*if no class that the student takes is in ts => counts++*/
            unsigned *student_classes = (unsigned *)malloc(2 * pi->mod_prefs[i].nmods * sizeof(unsigned));
            int count_classes = 0;
            for (k = 0; k < pi->mod_prefs[i].nmods; k++)
            {
                if (student_schedule[i][k])
                {
                    unsigned mid = pi->mod_prefs[i].mods[k].id;
                    unsigned c1 = 2 * mid, c2 = 2 * mid - 1;

                    student_classes[count_classes++] = c1;
                    student_classes[count_classes++] = c2;
                }
            }

            int class_in_ts = 0;
            for (k = 0; k < count_classes; k++)
            {
                for (r = 0; r < pi->R; r++)
                {
                    if (gene[r][ts_idx] == student_classes[k])
                    {
                        class_in_ts = 1;
                        break;
                    }
                }
            }

            if (class_in_ts)
                counts++;

            free(student_classes);
        }
    }

    obj[0] = counts;
}

/*FO2: cantidad de modulos preferidos no asignados*/
void countModRequestsMet(unsigned **students_schedule, double *obj, problem_instance *pi)
{
    int i, j;
    unsigned long counts = 0;

    for (i = 0; i < pi->S; i++)
    {
        unsigned n_required = pi->mod_prefs[i].nmods, met = 0;
        for (j = 0; j < n_required; j++)
            met += students_schedule[i][j];

        counts += n_required - met;
    }

    obj[1] = counts;
}

void check_room_cap(individual *ind, problem_instance *pi)
{
    int r, t, s, m;

    for (r = 0; r < pi->R; r++)
    {
        unsigned room_cap = pi->room_cap[r];
        /*get count of students that attend a class in r per time slot*/
        /**/
        unsigned long n_students = 0;
        for (t = 0; t < pi->T; t++)
        {
            unsigned cid = ind->gene[r][t];
            for (s = 0; s < pi->S; s++)
            {
                for (m = 0; m < pi->mod_prefs[s].nmods; m++)
                {
                    if (ind->student_modules[s][m])
                    {
                        unsigned c1 = 2 * pi->mod_prefs[s].mods[m].id;
                        unsigned c2 = 2 * pi->mod_prefs[s].mods[m].id - 1;

                        if (c1 == cid || c2 == cid)
                        {
                            n_students++;
                            break;
                        }
                    }
                }
            }
        }

        if (n_students > room_cap)
            ind->constr_violation--;
    }
}

void check_class_lim(individual *ind, problem_instance *pi)
{
    int c, s, m;
    for (c = 0; c < pi->C; c++)
    {
        unsigned class_lim = pi->class_lim[c];
        unsigned long class_count = 0;
        for (s = 0; s < pi->S; s++)
        {
            for (m = 0; m < pi->mod_prefs[s].nmods; m++)
            {
                if (ind->student_modules[s][m])
                {
                    unsigned c1 = 2 * pi->mod_prefs[s].mods[m].id;
                    unsigned c2 = 2 * pi->mod_prefs[s].mods[m].id - 1;

                    if (c1 == c + 1 || c2 == c + 1)
                    {
                        class_count++;
                        break;
                    }
                }
            }
        }

        if (class_count > class_lim)
            ind->constr_violation--;
    }
}

void check_min_mods(individual *ind, problem_instance *pi)
{
    int s, m;
    for (s = 0; s < pi->S; s++)
    {
        unsigned min_mods = pi->kmins[s];
        unsigned mods = 0;
        for (m = 0; m < pi->mod_prefs[s].nmods; m++)
            if (ind->student_modules[s][m])
                mods++;

        if (mods < min_mods)
            ind->constr_violation--;
    }
}

void check_max_mods(individual *ind, problem_instance *pi)
{
    int s, m;
    for (s = 0; s < pi->S; s++)
    {
        unsigned max_mods = pi->kmaxs[s];
        unsigned mods = 0;
        for (m = 0; m < pi->mod_prefs[s].nmods; m++)
            if (ind->student_modules[s][m])
                mods++;

        if (mods > max_mods)
            ind->constr_violation--;
    }
}

void test_problem(individual *ind, problem_instance *pi)
{
    /*objective functions*/
    countTimesRequestsMet(ind->student_modules, ind->gene, ind->obj, pi);
    countModRequestsMet(ind->student_modules, ind->obj, pi);

    /*constraint handling*/
    ind->constr_violation = 0;
    check_room_cap(ind, pi);
    check_class_lim(ind, pi);
    check_min_mods(ind, pi);
    check_max_mods(ind, pi);
}

/* Routine to evaluate objective function values and constraints for an individual */
void evaluate_ind(individual *ind, const problem_instance *pi)
{

    test_problem(ind, pi);

    return;
}
