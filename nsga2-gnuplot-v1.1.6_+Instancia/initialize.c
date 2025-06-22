/* Data initializtion routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

/* Function to initialize a population randomly */
void initialize_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        initialize_ind(&(pop->ind[i]), pi);
    }
    return;
}

int student_preinscribed_module(unsigned s, unsigned m, problem_instance *pi)
{
    unsigned i = 0;
    for (i = 0; i < pi->mod_prefs[s].nmods; i++)
    {
        if (pi->mod_prefs[s].mods[i].id - 1 == m)
            return 1;
    }
    return 0;
}

int student_preinscribed_class(unsigned s, unsigned c, problem_instance *pi)
{
    unsigned i, j;
    for (i = 0; i < pi->mod_prefs[s].nmods; i++)
    {
        unsigned m = pi->mod_prefs[s].mods[i].id;
        for (j = 0; j < 2; j++)
        {
            if (pi->mod_classes[m].classes[j].id == c + 1)
                return 1;
        }
    }
    return 0;
}

int student_has_timeslot_conflict(unsigned s, unsigned c, individual *ind, problem_instance *pi)
{
    char tslot_c[10] = ind->gene[c].ts;
    unsigned c2;

    for (c2 = 0; c2 < pi->C; c2++)
    {
        if (c2 == c)
            continue;

        // Si c2 no es una clase que el estudiante haya preinscrito, la ignoro
        if (!student_preinscribed_class(s, c2, pi))
            continue;

        char tslot_c2[10] = ind->gene[c2].ts;
        if (strcmp(tslot_c2, tslot_c) == 0)
            return 1; // conflicto detectado
    }
    return 0; // sin conflictos
}

int student_attends_class(unsigned s, unsigned c, individual *ind, problem_instance *pi)
{
    if (!student_preinscribed_class(s, c, pi))
        return 0; // no asiste si no preinscribió módulo asociado

    if (student_has_timeslot_conflict(s, c, ind, pi))
        return 0; // no asiste si hay conflicto horario

    return 1; // asiste si cumple ambas condiciones
}

int student_has_conflict(unsigned s, unsigned m, individual *ind, problem_instance *pi)
{
    unsigned j;
    for (j = 0; j < 2; j++)
    {                                                        // asumiendo siempre 2 clases por módulo
        unsigned cid = pi->mod_classes[m].classes[j].id - 1; // restar 1 para indexar en C
        if (student_has_timeslot_conflict(s, cid, ind, pi))
            return 1; // Conflicto encontrado
    }
    return 0; // Sin conflictos en ninguna clase del módulo
}

/* Function to initialize an individual randomly */
void initialize_ind(individual *ind, problem_instance *pi)
{
    int i, j, k;

    for (i = 0; i < pi->C; i++)
    {
        unsigned n_adq_rooms = pi->adqte_rooms[i].nrooms;

        if (n_adq_rooms == 0)
        {
            fprintf(stderr, "Error: clase %u no tiene salones adecuados\n", i);
            exit(EXIT_FAILURE);
        }

        unsigned adq_room_idx = rand() % n_adq_rooms;
        ind->gene[i].rid = pi->adqte_rooms[i].rooms[adq_room_idx].id;

        unsigned tsidx = rand() % pi->T;
        strcpy(ind->gene[i].ts, pi->tslots[tsidx].ts);
    }

    for (i = 0; i < pi->S; i++)
    {
        for (j = 0; j < pi->mod_prefs[i].nmods; j++)
        {
            unsigned mid = pi->mod_prefs[i].mods[j].id;
            if (!student_has_conflict(i, mid - 1, ind->gene, pi))
                ind->student_modules[i][mid - 1] = 1;
            else
                ind->student_modules[i][mid - 1] = 0;
        }
    }

    return;
}
