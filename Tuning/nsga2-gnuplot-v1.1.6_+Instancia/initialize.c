/* Data initializtion routines */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>

# include "global.h"
# include "rand.h"
# include <stdbool.h>

int fullprint=1;
/* Function to initialize a population randomly */
void initialize_pop (population *pop, problem_instance *pi)
{
    int i;
    printf("Initializing population\n");
    for (i=0; i<popsize; i++)
    {
        //initialize_ind (&(pop->ind[i]));
        initialize_backtracking_ind(&(pop->ind[i]), pi);
        printf("Individual %d initialized\n", i);
    }
    return;
}


/* Function to initialize an individual randomly */
void initialize_ind (individual *ind)
{
    int j, k;
    if (nreal!=0)
    {
        for (j=0; j<nreal; j++)
        {
            ind->xreal[j] = rnd (min_realvar[j], max_realvar[j]);
        }
    }
    if (nbin!=0)
    {
        for (j=0; j<nbin; j++)
        {
            for (k=0; k<nbits[j]; k++)
            {
                if (randomperc() <= 0.5)
                {
                    ind->gene[j][k] = 0;
                }
                else
                {
                    ind->gene[j][k] = 1;
                }
            }
        }
    }
    return;
}

bool is_valid_assignment(individual *ind, problem_instance *pi, int day, int employee, int shift) {
    int num_employees = pi->num_employees;
    int horizon_length = pi->horizon_length;
    int num_shifts = pi->num_shifts;
    int consecutive_shifts = 0;
    int consecutive_off = 0;
    int *shift_ammount = (int *)malloc(num_shifts * sizeof(int));
    if (shift_ammount == NULL) return false;
    memset(shift_ammount, 0, num_shifts * sizeof(int)); // Initialize to zero

    int weekcount = 0;
    int total_minutes = 0;

    for (int i = 0; i <= day; i++) {
        int shift_id = ind->xreal[i * num_employees + employee];

        //Max shifts
        if (shift_id != -1) {
            shift_ammount[shift_id]++;
            if (shift_ammount[shift_id] > pi->employees[employee].max_shifts[shift_id]) {
                free(shift_ammount);
                return false;
            }
        }
        //Check if the shift is compatible with the previous one
        if (i != 0) {
            int prev_shift = ind->xreal[(i - 1) * num_employees + employee];
            for (int j = 0; j < pi->shifts[prev_shift].num_incompatible_shifts; j++) {
                if (pi->shifts[prev_shift].incompatible_shifts[j] == shift_id) {
                    free(shift_ammount);
                    return false;
                }
            }
        }
        //Check if the employee is respecting the min consecutive shifts and days off
        if ((shift_id == 0 && consecutive_shifts > 0 && consecutive_shifts < pi->employees[employee].min_consecutive_shifts) ||
            (shift_id != 0 && consecutive_off > 0 && consecutive_off < pi->employees[employee].min_consecutive_days_off)) {
            free(shift_ammount);
            return false;
        }

        if (shift_id != 0) {
            consecutive_shifts++;
            consecutive_off = 0;
        } else {
            consecutive_shifts = 0;
            consecutive_off++;
        }

        if (consecutive_shifts > pi->employees[employee].max_consecutive_shifts) {
            free(shift_ammount);
            return false;
        }
        //Check if the employee is respecting the max total minutes
        if (i % 7 == 5) {
            if (shift_id != 0) {
                weekcount++;
            } else {
                if (i + 1 < horizon_length) {
                    int next_shift = ind->xreal[(i + 1) * num_employees + employee];
                    if (next_shift != 0) {
                        weekcount++;
                    }
                }
            }
        }
        //Count shifts and minutes
        total_minutes += pi->shifts[shift_id].length;
        if (total_minutes > pi->employees[employee].max_total_minutes) {
            free(shift_ammount);
            return false;
        }
    }
    //Check if the employee is respecting the max weekends
    if (weekcount > pi->employees[employee].max_weekends) {
        free(shift_ammount);
        return false;
    }

    int max_shift_length = 0;
    for (int i = 0; i < pi->num_shifts; i++) {
        if (pi->shifts[i].length > max_shift_length) {
            max_shift_length = pi->shifts[i].length;
        }
    }
    //Check if the employee is respecting the min total minutes
    if (total_minutes < pi->employees[employee].min_total_minutes) {
        int remaining_days = horizon_length - 1 - day;
        int max_remaining_minutes = remaining_days * max_shift_length;
        if (total_minutes + max_remaining_minutes < pi->employees[employee].min_total_minutes) {
            free(shift_ammount);
            return false;
        }
    }

    free(shift_ammount);
    return true;
}


bool backtrack(individual *ind, problem_instance *pi, int day, int employee) {
    if (day == pi->horizon_length) {
        return backtrack(ind, pi, 0, employee + 1); // Move to the next employee
    }
    
    if (employee == pi->num_employees) {
        return true; // All employees have been assigned
    }
    
    // Generate the list of possible shifts (excluding 0 for now)
    int num_possible_shifts = pi->num_shifts - 1;
    int possible_shifts[num_possible_shifts];
    for (int i = 1; i <= num_possible_shifts; i++) {
        possible_shifts[i - 1] = i;
    }

    // Shuffle the list of possible shifts
    shuffle(possible_shifts, num_possible_shifts);

    // Try assigning each shift in random order
    for (int i = 0; i < num_possible_shifts; i++) {
        int shift = possible_shifts[i];
        ind->xreal[day * pi->num_employees + employee] = shift;
        if (max_realvar[day * pi->num_employees + employee] < shift) {
            continue;
        }
        
        if (is_valid_assignment(ind, pi, day, employee, shift)) {
            if (backtrack(ind, pi, day + 1, employee)) {
                return true; // Found a valid assignment
            }
        }
    }

    // Finally, try the empty shift (0)
    int shift = 0;
    ind->xreal[day * pi->num_employees + employee] = shift;
    if (is_valid_assignment(ind, pi, day, employee, shift)) {
        if (backtrack(ind, pi, day + 1, employee)) {
            return true; // Found a valid assignment
        }
    }
    
    ind->xreal[day * pi->num_employees + employee] = -1; // Reset assignment
    return false; // No valid assignment found
}

void initialize_backtracking_ind(individual *ind, problem_instance *pi) {
    // Initialize all assignments to -1 (unassigned)
    for (int i = 0; i < pi->horizon_length * pi->num_employees; i++) {
        ind->xreal[i] = -1;
    }
    
    // Start backtracking from day 0, employee 0
    if (!backtrack(ind, pi, 0, 0)) {
        printf("Failed to find a valid initial assignment\n");
        // Handle failure case (e.g., by using a different initialization method)
    }
    fullprint=0;
    
    
}






