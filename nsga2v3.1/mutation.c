/* Mutation routines */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

#define SWAP(x, y)        \
    do                    \
    {                     \
        typeof(x) _x = x; \
        typeof(y) _y = y; \
        x = _y;           \
        y = _x;           \
    } while (0)

#define NORM2D(x, y) \
    (x * x) + (y * y)

typedef struct
{
    t_activity picked_act;
    size_t room_id;
} t_activity_choice;

/* Function to perform mutation in a population */
void mutation_pop(population *pop, problem_instance *pi)
{
    int i;
    for (i = 0; i < popsize; i++)
    {
        double p_mut = randomperc();
        if (p_mut < pmut_bin)
            mutation_ind(&(pop->ind[i]), pi);
    }
    return;
}

size_t roulette_timeslot(timeslot_counter *ts_counter, int size)
{
    double total = 0, cumprob = 0;
    int i;
    for (i = 0; i < size; i++)
        total += ts_counter[i].counter;

    if (total == 0)
        total = pow(2, -52);

    double probs[size];
    for (i = 0; i < size; i++)
        probs[i] = ts_counter[i].counter / total;

    double r = randomperc();

    for (i = 0; i < size; i++)
    {
        cumprob += probs[i];
        if (cumprob > r)
            return ts_counter[i].timeslot_idx;
    }

    return ts_counter[0].timeslot_idx;
}

void encode(individual *src, t_color *dst, problem_instance *pi)
{
    for (int a = 0; a < pi->nm_Activity; a++)
    {
        t_cellTuple *act_cell = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
        act_in_ind(pi, src, pi->A[a], act_cell);
        if (act_cell != NULL)
        {
            dst[a] = (t_color){.r = act_cell->r, .t = act_cell->t};
        }
        else
        {
            fprintf(stderr, "Error: not all acivities are in gene. Exiting with failure.\n");
            exit(EXIT_FAILURE);
        }

        free(act_cell);
    }
}

void decode(t_color *src, individual *dst, problem_instance *pi)
{
    for (int a = 0; a < pi->nm_Activity; a++)
    {
        dst->gene[src[a].r][src[a].t] = pi->A[a];
    }
}

int is_swap_feasible(int a1, int a2, t_color *encoded_gene, problem_instance *pi)
{
    size_t room_a = encoded_gene[a1].r;
    size_t room_b = encoded_gene[a2].r;

    int is_feasible_a = 0;

    for (int r1 = 0; r1 < pi->Ra[a1].nm_rooms; r1++)
    {
        if (pi->Ra[a1].rooms[r1].id - 1 == room_b)
        {
            is_feasible_a = 1;
            break;
        }
    }

    int is_feasible_b = 0;

    for (int r2 = 0; r2 < pi->Ra[a2].nm_rooms; r2++)
    {
        if (pi->Ra[a2].rooms[r2].id - 1 == room_a)
        {
            is_feasible_b = 1;
            break;
        }
    }

    return is_feasible_a && is_feasible_b;
}

void hc_swap(individual *ind, problem_instance *pi)
{
    evaluate_ind(ind, pi);
    long double current_fit = NORM2D(ind->obj[0], ind->obj[1]);
    int improved = 1, max_restarts = 5;

    t_color *encoded_gene = (t_color *)calloc(pi->nm_Activity, sizeof(t_color));

    // first improvement approach
    while (improved && max_restarts > 0)
    {
        improved = 0;
        for (int a = 0; a < pi->nm_Activity; a++)
        {
            for (int a_ = a + 1; a_ < pi->nm_Activity; a_++)
            {
                encode(ind, encoded_gene, pi);

                if (is_swap_feasible(a, a_, encoded_gene, pi) == 0)
                {
                    decode(encoded_gene, ind, pi);
                    continue;
                }

                SWAP(encoded_gene[a], encoded_gene[a_]);
                decode(encoded_gene, ind, pi);
                evaluate_ind(ind, pi);
                long double fit = NORM2D(ind->obj[0], ind->obj[1]);

                if (fit < current_fit)
                {
                    current_fit = fit;
                    improved = 1;
                    max_restarts--;
                    break;
                }
                else
                {
                    encode(ind, encoded_gene, pi);
                    SWAP(encoded_gene[a], encoded_gene[a_]);
                    decode(encoded_gene, ind, pi);
                }
            }
            if (improved)
                break;
        }
    }
    evaluate_ind(ind, pi);
    free(encoded_gene);
}

/* Function to perform mutation of an individual */
void mutation_ind(individual *ind, problem_instance *pi)
{
    // get array with most conflicted timeslots sorted from worst to best

    timeslot_counter *ts_counter = (timeslot_counter *)calloc(pi->nm_TimeSlots, sizeof(timeslot_counter));
    get_most_conflicted_free_timeslot(pi, ind, ts_counter);

    // split methods according to random chance

    int n_tslots_to_consider = 10, i;

    timeslot_counter worst_tslots[n_tslots_to_consider];
    timeslot_counter best_tslots[n_tslots_to_consider];

    for (i = 0; i < n_tslots_to_consider; i++)
        // copy top n_tslots_to_consider worst timeslots
        worst_tslots[i] = ts_counter[i];

    for (i = pi->nm_TimeSlots - 1; i >= pi->nm_TimeSlots - n_tslots_to_consider; i--)
        // copy top n_tslots_to_consider best timeslots (since ts_counter is sorted, we have to select from the last one)
        best_tslots[i % n_tslots_to_consider] = ts_counter[i];

    int t1 = roulette_timeslot(worst_tslots, n_tslots_to_consider);
    int t2 = roulette_timeslot(best_tslots, n_tslots_to_consider);

    double r = randomperc();

    if (r < 0.25)
    {
        // swap t1 with t2
        for (int r = 0; r < pi->nm_Rooms; r++)
        {
            t_activity tmp = ind->gene[r][t1];
            ind->gene[r][t1] = ind->gene[r][t2];
            ind->gene[r][t2] = tmp;
        }
    }
    else if (r < 0.5)
    {
        // move a random activity from t1 to t2
        int act = rnd(0, pi->nm_Activity - 1);
        t_cellTuple *act_cell = (t_cellTuple *)calloc(1, sizeof(t_cellTuple));
        act_in_ind(pi, ind, pi->A[act], act_cell);

        size_t id_course = get_course_activity(pi, pi->A[act]) - 1;

        int *flag_tslots = (int *)calloc(pi->nm_TimeSlots, sizeof(int));
        flag_tslots[act_cell->t] = 1;
        int a, t, r;
        for (a = 0; a < pi->Ac[id_course].nm_activities; a++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
                for (r = 0; r < pi->nm_Rooms; r++)
                    if (strcmp(ind->gene[r][t].id, pi->Ac[id_course].activities[a].id) == 0 && t != act_cell->t)
                        flag_tslots[t] = 1;
        }

        if (act_cell != NULL)
        {
            int moved = 0, count = 0;
            while (!moved)
            {
                if (strcmp(ind->gene[act_cell->r][t2].id, EmptyActivity.id) == 0 && flag_tslots[t2] == 0)
                {
                    ind->gene[act_cell->r][t2] = ind->gene[act_cell->r][act_cell->t];
                    ind->gene[act_cell->r][act_cell->t] = EmptyActivity;
                    moved = 1;
                    break;
                }
                else if (count < n_tslots_to_consider)
                    t2 = roulette_timeslot(best_tslots, n_tslots_to_consider);
                else
                    t2 = (t2 + 1) % pi->nm_TimeSlots;

                count++;
            }
        }

        free(act_cell);
        free(flag_tslots);
    }

    else
    {
        hc_swap(ind, pi);
    }

    free(ts_counter);
    return;
}