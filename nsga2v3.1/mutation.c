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
    t_cellTuple cell;
    for (int a = 0; a < pi->nm_Activity; a++)
    {
        if (act_in_ind(pi, src, pi->A[a], &cell))
        {
            dst[a] = (t_color){.r = cell.r, .t = cell.t};
        }
        else
        {
            fprintf(stderr, "Error: not all acivities are in gene. Exiting with failure.\n");
            exit(EXIT_FAILURE);
        }
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

size_t ls_fitness(problem_instance* pi, t_color* encoded_gene, int *student_busy_in_ts) {
    
    size_t count_conflict = 0;
    for (int s = 0; s < pi->nm_Students; s++){
        memset(student_busy_in_ts, 0, pi->nm_TimeSlots * sizeof(int));
        
        for (int c = 0; c < pi->Cs[s].nm_courses; c++) {
            size_t course_idx = pi->Cs[s].courses[c].id - 1;
            for (int a = 0; a < pi->Ac[course_idx].nm_activities; a++) {
                size_t act_idx = get_act_idx(pi, pi->Ac[course_idx].activities[a]);
                if (student_busy_in_ts[encoded_gene[act_idx].t] > 0)
                    count_conflict++;
                student_busy_in_ts[encoded_gene[act_idx].t]++;
            }
        }
    }

    return count_conflict;
}

void hc_swap(individual *ind, problem_instance *pi)
{
    int improved = 1, max_restarts = 10;

    /* transform to GCP rep */
    t_color *encoded_gene = (t_color *)calloc(pi->nm_Activity, sizeof(t_color));
    encode(ind, encoded_gene, pi);
    int * student_busy_in_ts = (int*) calloc(pi->nm_TimeSlots, sizeof(int));
    size_t current_fit = ls_fitness(pi, encoded_gene, student_busy_in_ts);

    // first improvement approach
    while (improved && max_restarts > 0)
    {
        improved = 0;
        for (int a = 0; a < pi->nm_Activity; a++)
        {
            for (int a_ = a + 1; a_ < pi->nm_Activity; a_++)
            {

                // if swap is not feasible, skip
                if (is_swap_feasible(a, a_, encoded_gene, pi) == 0)
                    continue;

                SWAP(encoded_gene[a], encoded_gene[a_]);
                size_t fit = ls_fitness(pi, encoded_gene, student_busy_in_ts);

                if (fit < current_fit)
                {
                    current_fit = fit;
                    improved = 1;
                    max_restarts--;
                    break;
                }
                /* rollback swap */
                else SWAP(encoded_gene[a], encoded_gene[a_]);
                
            }
            if (improved)
                break;
        }
    }
    /* go back to matrix rep */
    decode(encoded_gene,ind,pi);
    free(encoded_gene);
    free(student_busy_in_ts);
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

    double coin = randomperc();

    if (coin < 0.25)
    {
        // swap t1 with t2
        for (int r = 0; r < pi->nm_Rooms; r++)
        {
            t_activity tmp = ind->gene[r][t1];
            ind->gene[r][t1] = ind->gene[r][t2];
            ind->gene[r][t2] = tmp;
        }
    }
    else if (coin < 0.5)
    {
        // move a random activity from t1 to t2
        
        t_cellTuple act_cell;
        int worst_ts_acts[pi->nm_Rooms];
        int idx = 0;

        for (int r = 0; r < pi->nm_Rooms; r++)
            if (strcmp(ind->gene[r][t1].id, EmptyActivity.id) != 0)
                worst_ts_acts[idx++] = get_act_idx(pi,ind->gene[r][t1]);
        
        if (idx == 0) return;
        
        int id_act = worst_ts_acts[rnd(0, idx - 1)];
        act_in_ind(pi, ind, pi->A[id_act], &act_cell);

        size_t id_course = get_course_activity(pi, pi->A[id_act]) - 1;

        /* mark timeslots from the other activities belonging to the same course as the chosen one */
        int *flag_tslots = (int *)calloc(pi->nm_TimeSlots, sizeof(int));
        flag_tslots[t1] = 1;
        int a, t, r;
        for (a = 0; a < pi->Ac[id_course].nm_activities; a++)
        {
            for (t = 0; t < pi->nm_TimeSlots; t++)
                for (r = 0; r < pi->nm_Rooms; r++)
                    if (strcmp(ind->gene[r][t].id, pi->Ac[id_course].activities[a].id) == 0 && t != act_cell.t)
                        flag_tslots[t] = 1;
        }

        // try to move id_act to some available and feasible space in t2
        int moved = 0, count = 0;
        int *ts_tabu = (int*) calloc(pi->nm_TimeSlots, sizeof(int));

        // if t2 is already infeasible, perform RWS until is not
        if (flag_tslots[t2]) ts_tabu[t2] = 1;
        while (flag_tslots[t2] && count < pi->nm_TimeSlots){
            t2 = roulette_timeslot(best_tslots, n_tslots_to_consider);
            count++;
            if (count >= n_tslots_to_consider)
                t2 = (t2+1) % pi->nm_TimeSlots;
        }

        count = 0;
        int attempts = 0;
        
        while (!moved && attempts < pi->nm_TimeSlots)
        {
            attempts++;
            // first try in the same room...
            if (strcmp(ind->gene[act_cell.r][t2].id, EmptyActivity.id) == 0)
            {
                ind->gene[act_cell.r][t2] = ind->gene[act_cell.r][act_cell.t];
                ind->gene[act_cell.r][act_cell.t] = EmptyActivity;
                moved = 1;
                break;
            }

            // if it cant be done, try another room...
            for (int r = 0; r < pi->Ra[id_act].nm_rooms; r++) {
                if (strcmp(ind->gene[pi->Ra[id_act].rooms[r].id - 1][t2].id, EmptyActivity.id) == 0){
                    ind->gene[pi->Ra[id_act].rooms[r].id - 1][t2] = ind->gene[act_cell.r][act_cell.t];
                    ind->gene[act_cell.r][act_cell.t] = EmptyActivity;
                    moved = 1;
                    break;
                }
            }

            if (moved) break;

            // if all that fails, update t2 and add the previuos value to tabu list
            ts_tabu[t2] = 1;
            int loop_ward = 0;
            while ((flag_tslots[t2] || ts_tabu[t2]) && loop_ward < pi->nm_TimeSlots) {
                t2 = roulette_timeslot(best_tslots, n_tslots_to_consider);
                if (count >= n_tslots_to_consider)
                    t2 = (t2+1) % pi->nm_TimeSlots; 
                count++;
                loop_ward++;
            }


        }

        free(flag_tslots);
        free(ts_tabu);
    }

    else
    {
        hc_swap(ind, pi);
    }

    free(ts_counter);
    return;
}