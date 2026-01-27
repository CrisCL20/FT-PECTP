/* This file contains the variable and function declarations */
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdlib.h>

#define INF 1.0e14
#define EPS 1.0e-14
#define E 2.71828182845905
#define PI 3.14159265358979
#define GNUPLOT_COMMAND "gnuplot -persist"

typedef struct lists
{
    int index;
    struct lists *parent;
    struct lists *child;
} list;

/*
 *
 * @note: modify these structs to cttp instance
 *
 */

typedef struct
{
    unsigned id;
} t_student;

typedef struct
{
    char id[10];
} t_activity;

static const t_activity EmptyActivity = {.id = '\0'};

typedef struct
{
    unsigned id;
} t_course;

typedef struct
{
    char ts[10];
} t_timeslot;

typedef struct
{
    unsigned id;
} t_room;

typedef struct
{
    t_course *courses;
    size_t nm_courses;
} course_preference;

typedef struct
{
    t_room *rooms;
    size_t nm_rooms;
} adequate_rooms;

typedef struct
{
    t_timeslot *timeslots;
    size_t nm_timeslots;
} timeslot_preference;

typedef struct
{
    t_activity *activities;
    size_t nm_activities;
} course_activities;

typedef struct
{
    int rank;
    double constr_violation;
    t_activity **gene;
    unsigned **student_courses; /*Cursos en los que queda inscrito cada estudiante*/
    double *obj;
    double crowd_dist;
} individual;

typedef struct
{
    individual *ind;
} population;

typedef struct
{
    unsigned *rho;         /*size |R|*/
    unsigned *sigma_class; /* size |C|*/
    unsigned *kmins;       /* size |S| */
    unsigned *kmaxs;       /* size |S| */

    t_student *S;
    t_activity *A;
    t_course *C;
    t_timeslot *T;
    t_room *R;
    course_activities *Ac;
    adequate_rooms *Ra;
    course_preference *Cs;
    timeslot_preference *Ts;

    unsigned nm_Students, nm_Courses, nm_Activity, nm_TimeSlots, nm_Rooms;
} problem_instance;

/*****************/
/*****************/
/*****************/
/*****************/
/*****************/

extern int nreal;
extern int nbin;
extern int nobj;
extern int ncon;
extern int popsize;
extern double pcross_real;
extern double pcross_bin;
extern double pmut_real;
extern double pmut_bin;
extern double eta_c;
extern double eta_m;
extern int ngen;
extern int nbinmut;
extern int nrealmut;
extern int nbincross;
extern int nrealcross;
extern int *nbits;
extern double *min_realvar;
extern double *max_realvar;
extern double *min_binvar;
extern double *max_binvar;
extern int bitlength;
extern int choice;
extern int obj1;
extern int obj2;
extern int obj3;
extern int angle1;
extern int angle2;

int readInputFile(char *filePath, problem_instance *pi);
void printProblemInstance(problem_instance *pi);

void assign_students(individual *ind, problem_instance *pi);
void set_modules_matrix(individual *ind, unsigned **mat, problem_instance *pi);

char **str_split(char *a_str, const char a_delim);
int calculate_ts_idx(unsigned d, unsigned b1, unsigned T);
size_t get_act_idx(problem_instance *pi, t_activity a);
int get_timeslot_idx(problem_instance *pi, t_timeslot timeslot);
size_t get_course_activity(problem_instance *pi, t_activity act);
int course_in_student_preference(problem_instance *pi, int s_idx, size_t cid);

void allocate_memory_pop(population *pop, int size, problem_instance *pi);
void allocate_memory_ind(individual *ind, problem_instance *pi);
void deallocate_memory_pop(population *pop, int size, problem_instance *pi);
void deallocate_memory_ind(individual *ind, problem_instance *pi);

double maximum(double a, double b);
double minimum(double a, double b);

void crossover(individual *parent1, individual *parent2, individual *child1, individual *child2, problem_instance *pi);

void assign_crowding_distance_list(population *pop, list *lst, int front_size);
void assign_crowding_distance_indices(population *pop, int c1, int c2);
void assign_crowding_distance(population *pop, int *dist, int **obj_array, int front_size);

void decode_pop(population *pop);
void decode_ind(individual *ind);

void onthefly_display(population *pop, FILE *gp, int ii);

int check_dominance(individual *a, individual *b);

void evaluate_pop(population *pop, problem_instance *pi);
void evaluate_ind(individual *ind, problem_instance *pi);

void fill_nondominated_sort(population *mixed_pop, population *new_pop);
void crowding_fill(population *mixed_pop, population *new_pop, int count, int front_size, list *cur);

void initialize_pop(population *pop, problem_instance *pi);
void initialize_ind(individual *ind, problem_instance *pi);

void insert(list *node, int x);
list *del(list *node);

void merge(population *pop1, population *pop2, population *pop3);
void copy_ind(individual *ind1, individual *ind2);

void mutation_pop(population *pop, problem_instance *pi);
void mutation_ind(individual *ind, problem_instance *pi);

void test_problem(individual *ind, problem_instance *pi);

void assign_rank_and_crowding_distance(population *new_pop);

void report_pop(population *pop, FILE *fpt);
void report_feasible(population *pop, FILE *fpt, double elapsed);
void report_ind(individual *ind, FILE *fpt);

void quicksort_front_obj(population *pop, int objcount, int obj_array[], int obj_array_size);
void q_sort_front_obj(population *pop, int objcount, int obj_array[], int left, int right);
void quicksort_dist(population *pop, int *dist, int front_size);
void q_sort_dist(population *pop, int *dist, int left, int right);

void selection(population *old_pop, population *new_pop, problem_instance *pi);
individual *tournament(individual *ind1, individual *ind2);

#endif