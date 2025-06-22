/* Routine for evaluating population members  */

/****
 * 
 *  @note: modify this with ucttp format
 * 
 */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>
# include "global.h"
# include "rand.h"

void printProblemInstance(problem_instance *pi){
    int i, j;
    
    printf("Estudiantes: %d\n", pi->S);
    for (i=0; i<pi->S; i++){
        printf("Módulos preferidos por el estudiante %d:\n", pi->students[i].id);
        for (j = 0; j < pi->mod_prefs[i].nmods; j++)
            printf("%d ", pi->mod_prefs[i].mods[j].id);
        printf("\n");
    }

    for(i = 0; i < pi->S; i++) {
        printf("Bloques de tiempo preferidos por el estudiante %d:\n", pi->students[i].id);

        for (j=0; j < pi->tslot_prefs[i].nprefs; j++) 
            printf("%s ", pi->tslot_prefs[i].tslots[j].ts);
        
        printf("\n");
    }

    for (i = 0; i < pi->S; i++) {
        printf("El estudiante %d debe tomar a lo menos %d y a lo más %d ramos.\n", 
            pi->students[i].id,
            pi->kmins[i],
            pi->kmaxs[i]
        );
    }

    printf("Clases: %d\n", pi->C);
    for(i=0; i < pi->C; i++)
        printf("La clase %d tiene un límite de %d.\n", pi->classes[i].id, pi->class_lim[i]);
    
    for(i = 0; i < pi->C; i++) {
        printf("Salones que son adecuados para realizar la clase %d:\n", pi->classes[i].id);
        for(j = 0; j < pi->adqte_rooms[i].nrooms; j++) 
            printf("%d ", pi->adqte_rooms[i].rooms[j].id);
        printf("\n");
    }

    printf("Modulos: %d\n", pi->M);
    for(i=0; i < pi->M;i++)
        printf("El módulo %d requiere asistir a las clases %d y %d.\n", 
            pi->modules[i].id,
            pi->mod_classes[i].classes[0].id,
            pi->mod_classes[i].classes[1].id
        );

    return;

}
