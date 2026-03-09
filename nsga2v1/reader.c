/* Routine for evaluating population members  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "global.h"
#include "rand.h"

void findDef(FILE *f, char *def)
{
    char word[4096];
    /* assumes no word exceeds length of 1023 */
    while (fscanf(f, " %4096s\n", word))
    {
        if (strcmp(word, def) == 0)
            break;
    }
}

void removeSemicolon(char *line)
{
    strtok(line, ";");
}

int countWords(char *line)
{
    int words;
    char linet[4096], *token;
    strcpy(linet, line);

    words = 0;
    token = strtok(linet, " ");

    while (token != NULL)
    {
        words++;
        token = strtok(NULL, " ");
    }

    free(token);
    return words;
}

void readStudents(FILE *fh, problem_instance *pi)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);
    removeSemicolon(line);
    if (debug)
        printf("Line: %s\n", line);

    pi->nm_Students = countWords(line);
    if (debug)
        printf("|S|: %d\n", pi->nm_Students);
    pi->S = (t_student *)malloc(pi->nm_Students * sizeof(t_student));
    pi->Cs = (course_preference *)malloc(pi->nm_Students * sizeof(course_preference));
    pi->Ts = (timeslot_preference *)malloc(pi->nm_Students * sizeof(timeslot_preference));
    pi->kmaxs = (unsigned int *)malloc(pi->nm_Students * sizeof(unsigned int));
    pi->kmins = (unsigned int *)malloc(pi->nm_Students * sizeof(unsigned int));

    token = strtok(line, " ");
    while (token != NULL)
    {
        if (debug)
            printf("Estudiante %s\n", token);
        pi->S[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        for (id = 0; id < pi->nm_Students; id++)
        {
            printf("%d\n", pi->S[id].id);
        }
    }

    free(token);
}

void readActivities(FILE *fh, problem_instance *pi)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);
    removeSemicolon(line);
    if (debug)
        printf("Line: %s\n", line);

    pi->nm_Activity = countWords(line);
    if (debug)
        printf("Size of activities: %d\n", pi->nm_Activity);
    pi->A = (t_activity *)malloc(pi->nm_Activity * sizeof(t_activity));
    pi->Ra = (adequate_rooms *)malloc(pi->nm_Activity * sizeof(adequate_rooms));

    token = strtok(line, " ");
    while (token != NULL)
    {
        if (debug)
            printf("Token: %s\n", token);
        strcpy(pi->A[id].id, token);

        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        for (id = 0; id < pi->nm_Activity; id++)
            printf("%s\n", pi->A[id].id);
    }

    free(token);
}

void readCourses(FILE *fh, problem_instance *pi)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);
    removeSemicolon(line);

    pi->nm_Courses = countWords(line);
    if (debug)
        printf("|C|: %d\n", pi->nm_Courses);
    pi->C = (t_course *)malloc(pi->nm_Courses * sizeof(t_course));
    pi->Ac = (course_activities *)malloc(pi->nm_Courses * sizeof(course_activities));
    pi->sigma_class = (unsigned int *)malloc(pi->nm_Courses * sizeof(unsigned int));

    token = strtok(line, " ");
    while (token != NULL)
    {
        pi->C[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        for (id = 0; id < pi->nm_Courses; id++)
            printf("Modulo %d\n", pi->C[id].id);
    }

    free(token);
}

void readRooms(FILE *fh, problem_instance *pi)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    removeSemicolon(line);

    pi->nm_Rooms = countWords(line);
    if (debug)
        printf("|R|: %d\n", pi->nm_Rooms);
    pi->R = (t_room *)malloc(pi->nm_Rooms * sizeof(t_room));
    pi->rho = (unsigned int *)malloc(pi->nm_Rooms * sizeof(unsigned int));

    token = strtok(line, " ");
    while (token != NULL)
    {
        pi->R[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        for (id = 0; id < pi->nm_Rooms; id++)
            printf("Salón %d\n", pi->R[id].id);
    }

    free(token);
}

void readTimeSlots(FILE *fh, problem_instance *pi)
{
    int debug = 0, id = 0, d = 0, i = 0;
    char *token;
    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    pi->nm_TimeSlots = 5 * countWords(line);
    if (debug)
        printf("|T|: %d\n", pi->nm_TimeSlots);
    pi->T = (t_timeslot *)malloc(pi->nm_TimeSlots * sizeof(t_timeslot));

    for (d = 0; d < 5; d++)
    {

        token = strtok(line, " ");
        while (token != NULL)
        {
            token[strcspn(token, "\n")] = 0;
            strcpy(pi->T[id].ts, token);
            token = strtok(NULL, " ");
            id++;
        }
        fgets(line, sizeof(line), fh);
        if (debug)
            printf("Line: %s\n", line);
    }

    if (debug)
    {
        for (i = 0; i < pi->nm_TimeSlots; i++)
            printf("Timeslot %s.\n", pi->T[i].ts);
    }

    free(token);
}

void readCourseActivities(FILE *fh, problem_instance *pi, unsigned cid)
{

    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    removeSemicolon(line);
    if (debug)
        printf("Line: %s.\n", line);

    pi->Ac[cid].nm_activities = countWords(line);
    if (debug)
        printf("|Ac[%d]| = %ld\n", pi->C[cid].id, pi->Ac[cid].nm_activities);
    pi->Ac[cid].activities = (t_activity *)malloc(pi->Ac[cid].nm_activities * sizeof(t_activity));

    token = strtok(line, " ");
    while (token != NULL)
    {
        strcpy(pi->Ac[cid].activities[id].id, token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        printf("Curso %d requiere las siguientes actividades:\n\t", pi->C[cid].id);
        for (unsigned i = 0; i < pi->Ac[cid].nm_activities; ++i)
            printf("%s ", pi->Ac[cid].activities[i].id);
        printf("\n");
    }

    free(token);
}

void readCoursePreference(FILE *fh, problem_instance *pi, unsigned sid)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    removeSemicolon(line);
    if (debug)
        printf("Line: %s\n", line);

    pi->Cs[sid].nm_courses = countWords(line);
    pi->Cs[sid].courses = (t_course *)malloc(pi->Cs[sid].nm_courses * sizeof(t_course));

    if (debug)
        printf("El estudiante %d tiene preferencia por %lu cursos.\n",
               pi->S[sid].id,
               pi->Cs[sid].nm_courses);

    token = strtok(line, " ");
    while (token != NULL)
    {
        pi->Cs[sid].courses[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        printf("El estudiante %d pre-inscribe los siguientes cursos:\n", pi->S[sid].id);
        for (id = 0; id < pi->Cs[sid].nm_courses; id++)
        {
            printf("%d ", pi->Cs[sid].courses[id].id);
        }
        printf("\n");
    }

    free(token);
}

void readAdequateRooms(FILE *fh, problem_instance *pi, unsigned aid)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    removeSemicolon(line);
    if (debug)
        printf("Line: %s", line);

    pi->Ra[aid].nm_rooms = countWords(line);
    pi->Ra[aid].rooms = (t_room *)malloc(pi->Ra[aid].nm_rooms * sizeof(t_room));

    token = strtok(line, " ");
    while (token != NULL)
    {
        pi->Ra[aid].rooms[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        printf("La actividad %s puede realizarse en los siguientes salones:\n", pi->A[aid].id);
        for (id = 0; id < pi->Ra[aid].nm_rooms; id++)
        {
            printf("%d ", pi->Ra[aid].rooms[id].id);
        }
        printf("\n");
    }

    free(token);
}

void readTimeSlotPreference(FILE *fh, problem_instance *pi, unsigned sid)
{
    int debug = 0, id = 0;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    removeSemicolon(line);
    if (debug)
        printf("Line: %s.\n", line);

    if (strstr(line, ";"))
    {
        pi->Ts[sid].nm_timeslots = 0;
    }
    else
    {
        pi->Ts[sid].nm_timeslots = countWords(line);
    }

    if (debug)
        printf("|Ts[%d]| = %ld\n", pi->S[sid].id, pi->Ts[sid].nm_timeslots);
    pi->Ts[sid].timeslots = (t_timeslot *)malloc(pi->Ts[sid].nm_timeslots * sizeof(t_timeslot));

    token = strtok(line, " ");
    while (token != NULL && pi->Ts[sid].nm_timeslots > 0)
    {
        strcpy(pi->Ts[sid].timeslots[id].ts, token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug)
    {
        printf("El estudiante %d tiene %ld preferencias horarias:\n\t", pi->S[sid].id, pi->Ts[sid].nm_timeslots);
        for (id = 0; id < pi->Ts[sid].nm_timeslots; id++)
        {
            printf("%s ", pi->Ts[sid].timeslots[id].ts);
        }
        printf(".\n");
    }

    if (pi->Ts[sid].nm_timeslots > 0)
        free(token);
}

void readRoomCapacity(FILE *fh, problem_instance *pi)
{
    int debug = 0, i = 0, rcap;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    for (i = 0; i < pi->nm_Rooms; i++)
    {
        token = strtok(line, " ");
        token = strtok(NULL, " ");
        rcap = atoi(token);

        pi->rho[i] = rcap;
        fgets(line, sizeof(line), fh);
        if (debug)
            printf("Line: %s\n", line);
    }

    if (debug)
    {
        for (i = 0; i < pi->nm_Rooms; i++)
            printf("Capacidad del salon %d: %d\n", pi->R[i].id, pi->rho[i]);
    }

    // free(token);
}

void ReadActivityLimit(FILE *fh, problem_instance *pi)
{
    int debug = 0, i = 0, clim;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    for (i = 0; i < pi->nm_Courses; i++)
    {
        token = strtok(line, " ");
        token = strtok(NULL, " ");
        clim = atoi(token);

        pi->sigma_class[i] = clim;
        fgets(line, sizeof(line), fh);
        if (debug)
            printf("Line: %s\n", line);
    }

    if (debug)
    {
        for (i = 0; i < pi->nm_Courses; i++)
            printf("Limite del curso %d: %d\n", pi->C[i].id, pi->sigma_class[i]);
    }

    // free(token);
}

void readKmin(FILE *fh, problem_instance *pi)
{
    int debug = 0, i = 0, sid, kmin;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    for (i = 0; i < pi->nm_Students; i++)
    {
        token = strtok(line, " ");
        sid = atoi(token);
        token = strtok(NULL, " ");
        kmin = atoi(token);

        pi->kmins[i] = kmin;
        if (debug)
            printf("Cursos mínimos para el estudiante %d: %d\n", pi->S[i].id, pi->kmins[sid - 1]);
        fgets(line, sizeof(line), fh);
        if (debug)
            printf("Line: %s\n", line);
    }

    if (debug)
    {
        for (i = 0; i < pi->nm_Students; i++)
            printf("Limite inferior para el estudiante %d: %d\n", pi->S[i].id, pi->kmins[i]);
    }

    // free(token);
}

void readKmax(FILE *fh, problem_instance *pi)
{
    int debug = 0, i = 0, sid, kmax;
    char *token;

    char line[4096];
    fgets(line, sizeof(line), fh);

    if (debug)
        printf("Line: %s\n", line);

    for (i = 0; i < pi->nm_Students; i++)
    {
        token = strtok(line, " ");
        sid = atoi(token);
        token = strtok(NULL, " ");
        kmax = atoi(token);

        pi->kmaxs[sid - 1] = kmax;
        fgets(line, sizeof(line), fh);
        if (debug)
            printf("Line: %s\n", line);
    }

    if (debug)
    {
        for (i = 0; i < pi->nm_Students; i++)
            printf("Limite superior para el estudiante %d: %d\n", pi->S[i].id, pi->kmaxs[i]);
    }

    // free(token);
}

int readInputFile(char *filePath, problem_instance *pi)
{
    int debug = 1, i = 0;
    FILE *fh = fopen(filePath, "r");

    /*check if file exists*/
    if (fh == NULL)
    {
        printf("File does not exists %s", filePath);
        exit(1);
    }

    if (debug)
        printf("Reading: %s \n", filePath);

    /**
     *
     * @note: reemplazar definiciones por ucttp
     *
     **/

    if (debug)
        printf("Reading students...\n");
    findDef(fh, "S:=");
    readStudents(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading courses...\n");
    findDef(fh, "C:=");
    readCourses(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading activities...\n");
    findDef(fh, "A:=");
    readActivities(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading timeslots...\n");
    findDef(fh, "T:=");
    if (debug)
        printf("Found timeslots definition!\n");
    readTimeSlots(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading rooms...\n");
    findDef(fh, "R:=");
    readRooms(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading course activities...\n");
    for (i = 0; i < pi->nm_Courses; i++)
    {
        char Ac[128];
        sprintf(Ac, "Ac[%d]:=", pi->C[i].id);
        if (debug)
            printf("Reading activities for course %d...\n", pi->C[i].id);
        findDef(fh, Ac);
        readCourseActivities(fh, pi, i);
        if (debug)
            printf("END\n");
    }

    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading student course preferences...\n");
    for (i = 0; i < pi->nm_Students; i++)
    {
        char Cs[128];
        sprintf(Cs, "Cs[%d]:=", pi->S[i].id);
        findDef(fh, Cs);
        readCoursePreference(fh, pi, pi->S[i].id - 1);
    }

    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading activity room compatibility...\n");

    for (i = 0; i < pi->nm_Activity; i++)
    {
        char Rc[128];
        sprintf(Rc, "Ra[%s]:=", pi->A[i].id);
        findDef(fh, Rc);
        readAdequateRooms(fh, pi, i);
    }

    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading student time preference...\n");
    for (i = 0; i < pi->nm_Students; i++)
    {
        char Ts[128];
        sprintf(Ts, "Ts[%d]:=", pi->S[i].id);
        findDef(fh, Ts);
        readTimeSlotPreference(fh, pi, i);
    }

    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading room capacity...\n");
    findDef(fh, "rho:=");
    readRoomCapacity(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading class limit...\n");
    findDef(fh, "sigma_class:=");
    ReadActivityLimit(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading kmin...\n");
    findDef(fh, "kmin:=");
    readKmin(fh, pi);
    if (debug)
        printf("END\n");

    if (debug)
        printf("Reading kmax...\n");
    findDef(fh, "kmax:=");
    readKmax(fh, pi);
    if (debug)
        printf("END\n");

    /*************/
    /*************/
    /*************/
    /*************/
    /*************/
    /*************/
    /*************/

    fclose(fh);
    if (debug)
        printf("End Reading! \n");

    if (debug)
        printProblemInstance(pi);

    return 0;
}
