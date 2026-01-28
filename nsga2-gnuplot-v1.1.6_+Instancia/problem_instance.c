/* Routine for evaluating population members  */

/****
 *
 *  @note: modify this with ucttp format
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "global.h"
#include "rand.h"

void printProblemInstance(problem_instance *pi)
{
    int i, j;

    printf("Estudiantes: %d\n", pi->nm_Students);
    for (i = 0; i < pi->nm_Students; i++)
    {
        printf("Módulos preferidos por el estudiante (%ld) %d:\n", pi->Cs[i].nm_courses, pi->S[i].id);
        for (j = 0; j < pi->Cs[i].nm_courses; j++)
            printf("%d ", pi->Cs[i].courses[j].id);
        printf("\n");
    }

    for (i = 0; i < pi->nm_Students; i++)
    {
        printf("Bloques de tiempo preferidos por el estudiante %d:\n", pi->S[i].id);

        for (j = 0; j < pi->Ts[i].nm_timeslots; j++)
            printf("%s ", pi->Ts[i].timeslots[j].ts);

        printf("\n");
    }

    for (i = 0; i < pi->nm_Students; i++)
    {
        printf("El estudiante %d debe tomar a lo menos %d y a lo más %d ramos.\n",
               pi->S[i].id,
               pi->kmins[i],
               pi->kmaxs[i]);
    }

    printf("Cursos: %d\n", pi->nm_Courses);
    for (i = 0; i < pi->nm_Courses; i++)
    {
        printf("El curso %d tiene un límite de %d y requiere asistir las siguientes actividades:\n\t", pi->C[i].id, pi->sigma_class[i]);
        for (j = 0; j < pi->Ac[i].nm_activities; j++)
        {
            printf("%s ", pi->Ac[i].activities[j].id);
        }
        printf("\n");
    }

    printf("Actividades: %d\n\t", pi->nm_Activity);

    for (i = 0; i < pi->nm_Activity; i++)
    {
        printf("%s ", pi->A[i].id);
    }
    printf("\n");

    for (i = 0; i < pi->nm_Activity; i++)
    {
        printf("Salones que son adecuados para realizar la actividad %s:\n\t", pi->A[i].id);
        for (j = 0; j < pi->Ra[i].nm_rooms; j++)
            printf("%d ", pi->Ra[i].rooms[j].id);
        printf("\n");
    }

    return;
}
