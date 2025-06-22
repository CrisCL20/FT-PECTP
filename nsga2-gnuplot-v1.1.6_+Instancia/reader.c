/* Routine for evaluating population members  */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <string.h>
# include "global.h"
# include "rand.h"


void findDef(FILE *f, char *def) {
    char word[1024];
    /* assumes no word exceeds length of 1023 */
    while (fscanf(f, " %1023s\n", word)) {
        if(strcmp(word,def) == 0) break;
    }
}

void removeSemicolon(char *line){
    strtok(line, ";");
}

int countWords(char *line){
    int words;
    char linet[1024], *token;
    strcpy(linet, line);

    words = 0;
    token = strtok(linet, " ");

    while( token != NULL ) {
        words ++;
        token = strtok(NULL, " ");
    }
   return words;
}

void readStudents(FILE* fh, problem_instance* pi) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if (debug) printf("Line: %s\n",line);
    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);

    pi->S = countWords(line);
    if(debug) printf("Nstudents: %d\n", pi->S);
    pi->students = (student *) malloc(pi->S * sizeof(student));
    pi->mod_prefs = (module_preference *) malloc(pi->S * sizeof(module_preference));
    pi->tslot_prefs = (tslot_preference *) malloc(pi->S * sizeof(tslot_preference));
    pi->kmaxs = (unsigned int *) malloc(pi->S * sizeof(unsigned int));
    pi->kmins = (unsigned int *) malloc(pi->S * sizeof(unsigned int));

    token = strtok(line, " ");
    while (token != NULL) {
        if (debug) printf("Estudiante %s\n", token);
        pi->students[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug) {
        for(id = 0; id < pi->S; id++) {
            printf("%d\n", pi->students[id].id);
        }
    }

}

void readClasses(FILE* fh, problem_instance* pi) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if (debug) printf("Line: %s\n", line);
    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);

    pi->C = countWords(line);
    if (debug) printf("Size of classes: %d\n", pi->C);
    pi->classes = (class *) malloc(pi->C * sizeof(class));
    pi->adqte_rooms = (adequate_rooms *) malloc(pi->C * sizeof(adequate_rooms));
    pi->class_lim = (unsigned int *) malloc(pi->C * sizeof(unsigned int));

    token = strtok(line, " ");
    while(token != NULL) {
        if (debug) printf("Token: %s\n", token);
        pi->classes[id].id = atoi(token);

        token = strtok(NULL, " ");
        id++;
    }

    if(debug) {
        for(id = 0; id < pi->C; id++)
            printf("%d\n", pi->classes[id].id);
    }
}

void readModules(FILE* fh, problem_instance* pi) {
    int debug=0,id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);
    removeSemicolon(line);

    pi->M = countWords(line);
    if (debug) printf("Nmodulos: %d\n", pi->M);
    pi->modules = (module *) malloc(pi->M * sizeof(module));
    pi->mod_classes = (module_classes *) malloc(pi->M * sizeof(module_classes));
    token = strtok(line, " ");
    while (token != NULL)
    {
        pi->modules[id].id = atoi(token);
        token = strtok(NULL," ");
        id++;
    }

    if(debug) {
        for(id = 0; id < pi->M; id++)
            printf("Modulo %d\n", pi->modules[id].id);
    }
    
}

void readRooms(FILE* fh, problem_instance* pi) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);

    removeSemicolon(line);

    pi->R = countWords(line);
    if(debug) printf("Nsalones: %d\n", pi->R);
    pi->rooms = (room *) malloc(pi->R * sizeof(room));
    pi->room_cap = (unsigned int *) malloc(pi->R * sizeof(unsigned int));
    
    token = strtok(line, " ");
    while (token != NULL)   
    {
        pi->rooms[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if(debug) {
        for(id = 0; id < pi->R; id++)
            printf("Salón %d\n", pi->rooms[id].id);
    }
    

}

void readTimeslots(FILE* fh, problem_instance* pi) {
    int debug=0, id=0, d=0, i=0;
    char* token;
    char line[1024];
    fgets(line,sizeof(line), fh);
    
    if (debug) printf("Line: %s\n", line);

    pi->T = 5 * countWords(line);
    if (debug) printf("Size of T: %d\n",pi->T);
    pi->tslots = (tslot *) malloc(pi->T * sizeof(tslot));
    
    for(d=0; d < 5; d++){
        
        
        token = strtok(line," ");
        while(token != NULL) {
            token[strcspn(token,"\n")] = 0;
            strcpy(pi->tslots[id].ts, token);
            token = strtok(NULL, " ");
            id++;
        }
        fgets(line, sizeof(line), fh);
        if(debug) printf("Line: %s\n", line);
    }

    if (debug) {
        for(i = 0; i < pi->T; i++)
            printf("Timeslot %s\n", pi->tslots[i].ts);
    }
    
}

void readModuleClass(FILE* fh, problem_instance* pi, unsigned mid) {

    int debug=0,id=0;
    char* token;

    char line[1024];
    fgets(line, sizeof(line),fh);

    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);

    token = strtok(line, " ");

    while (token != NULL) {
        pi->mod_classes[mid].classes[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if(debug) {
        printf("Modulo %d requiere las clases C%d y C%d\n", 
            mid+1, 
            pi->mod_classes[mid].classes[0].id, 
            pi->mod_classes[mid].classes[1].id);
    }

}

void readModulePrefs(FILE* fh, problem_instance* pi, unsigned sid) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);

    pi->mod_prefs[sid].nmods = countWords(line);
    pi->mod_prefs[sid].mods = (module *) malloc(pi->mod_prefs[sid].nmods * sizeof(module));

    if (debug) printf("El estudiante %d tiene preferencia por %lu modulos.\n", 
        pi->students[sid].id,
        pi->mod_prefs[sid].nmods
    );

    token = strtok(line, " ");
    while(token != NULL) {
        pi->mod_prefs[sid].mods[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug) {
        printf("El estudiante %d pre-inscribe los siguientes módulos:\n", sid+1);
        for (id = 0; id < pi->mod_prefs[sid].nmods; id++) {
            printf("%d ", pi->mod_prefs[sid].mods[id].id);
        }
        printf("\n");
    }

}

void readAdqteRooms(FILE* fh, problem_instance* pi, unsigned cid) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    removeSemicolon(line);
    if (debug) printf("Line: %s", line);

    pi->adqte_rooms[cid].nrooms = countWords(line);
    pi->adqte_rooms[cid].rooms = (room *) malloc(pi->adqte_rooms[cid].nrooms * sizeof(room));

    token = strtok(line, " ");
    while(token != NULL) {
        pi->adqte_rooms[cid].rooms[id].id = atoi(token);
        token = strtok(NULL, " ");
        id++;
    }


    if (debug) {
        printf("La clase %d puede realizarse en los siguientes salones:\n", cid+1);
        for (id = 0; id < pi->adqte_rooms[cid].nrooms; id++) {
            printf("%d ", pi->adqte_rooms[cid].rooms[id].id);
        }
        printf("\n");
    }
}

void readTslotPrefs(FILE* fh, problem_instance* pi, unsigned sid) {
    int debug=0, id=0;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    removeSemicolon(line);
    if (debug) printf("Line: %s\n", line);

    pi->tslot_prefs[sid].nprefs = countWords(line);
    pi->tslot_prefs[sid].tslots = (tslot *) malloc(pi->tslot_prefs[sid].nprefs * sizeof(tslot));

    token = strtok(line, " ");
    while(token != NULL) {
        strcpy(pi->tslot_prefs[sid].tslots[id].ts, token);
        token = strtok(NULL, " ");
        id++;
    }

    if (debug) {
        printf("El estudiante %d tiene las siguientes preferencias horarias:\n", sid+1);
        for (id = 0; id < pi->tslot_prefs[sid].nprefs; id++) {
            printf("%s ", pi->tslot_prefs[sid].tslots[id].ts);
        }
        printf("\n");
    }
}

void readRoomCaps(FILE* fh, problem_instance* pi) {
    int debug=0, i=0, rcap;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);

    for( i=0; i < pi->R; i++){   
        token = strtok(line, " ");
        token = strtok(NULL, " ");
        rcap = atoi(token);

        pi->room_cap[i] = rcap;
        fgets(line,sizeof(line), fh);
        if (debug) printf("Line: %s\n", line);
    }

    if(debug) {
        for (i = 0; i < pi->R; i++)
            printf("Capacidad del salon %d: %d\n", i+1,pi->room_cap[i]);
    }
}

void ReadClassLims(FILE* fh, problem_instance* pi) {
    int debug=0, i =0, clim;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);

    for( i=0; i < pi->C; i++){   
        token = strtok(line, " ");
        token = strtok(NULL, " ");
        clim = atoi(token);

        pi->class_lim[i] = clim;
        fgets(line,sizeof(line), fh);
        if (debug) printf("Line: %s\n", line);
    }

    if(debug) {
        for (i = 0; i < pi->C; i++)
            printf("Limite de la clase %d: %d\n", i+1,pi->class_lim[i]);
    }
}

void readKmin(FILE* fh, problem_instance* pi) {
    int debug=0, id=0, i =0, sid, kmin;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);

    for( i=0; i < pi->S; i++){   
        token = strtok(line, " ");
        sid = atoi(token);
        token = strtok(NULL, " ");
        kmin = atoi(token);

        pi->kmins[i] = kmin;
        fgets(line,sizeof(line), fh);
        if (debug) printf("Line: %s\n", line);
    }

    if(debug) {
        for (i = 0; i < pi->S; i++)
            printf("Limite inferior para el estudiante %d: %d\n", i+1,pi->kmins[i]);
    }  
}

void readKmax(FILE* fh, problem_instance* pi) {
    int debug=0, id=0, i =0, sid, kmax;
    char* token;

    char line[1024];
    fgets(line,sizeof(line), fh);

    if(debug) printf("Line: %s\n", line);

    for( i=0; i < pi->S; i++){   
        token = strtok(line, " ");
        sid = atoi(token);
        token = strtok(NULL, " ");
        kmax = atoi(token);

        pi->kmaxs[i] = kmax;
        fgets(line,sizeof(line), fh);
        if (debug) printf("Line: %s\n", line);
    }

    if(debug) {
        for (i = 0; i < pi->S; i++)
            printf("Limite superior para el estudiante %d: %d\n", i+1,pi->kmaxs[i]);
    }  
}

int readInputFile(char* filePath, problem_instance *pi) {
    int debug=0, i = 0;
    FILE* fh=fopen(filePath, "r");

    /*check if file exists*/
    if ( fh==NULL ){
        printf("File does not exists %s", filePath);
        return 0;
    }

    if(debug) printf("Reading: %s \n", filePath);

    /**
    *
    * @note: reemplazar definiciones por ucttp
    * 
    **/

    findDef(fh, "S:=");
    readStudents(fh, pi);
    if(debug) printf("End readStudents! \n");

    findDef(fh, "C:=");
    readClasses(fh,pi);
    if (debug) printf("End readClasses!\n");

    findDef(fh, "M:=");
    readModules(fh,pi);
    if (debug) printf("End readModules!\n");

    findDef(fh, "T:=");
    readTimeslots(fh, pi);
    if (debug) printf("End readTimeslots!\n");

    findDef(fh, "R:=");
    readRooms(fh, pi);
    if (debug) printf("End readRooms!\n");
    
    for (i = 0; i < pi->M; i++) {
        char CM[128];
        sprintf(CM,"CM[%d]:=", pi->modules[i].id);
        findDef(fh, CM);
        readModuleClass(fh, pi, i);        
    }

    if (debug) printf("End readModuleClass!\n");

    for (i = 0; i < pi->S; i++) {
        char Ms[128];
        sprintf(Ms,"Ms[%d]:=", pi->students[i].id);
        findDef(fh, Ms);
        readModulePrefs(fh, pi, i);
    }

    if (debug) printf("End readModulePrefs!\n");

    for (i = 0; i < pi->C; i++) {
        char Rc[128];
        sprintf(Rc,"Rc[%d]:=", pi->classes[i].id);
        findDef(fh, Rc);
        readAdqteRooms(fh, pi, i);
    }

    if (debug) printf("End readAdqteRooms!\n");
    
    for (i = 0; i < pi->S; i++) {
        char PTs[128];
        sprintf(PTs,"PTs[%d]:=", pi->students[i].id);
        findDef(fh, PTs);
        readTslotPrefs(fh, pi, i);
    }

    if (debug) printf("End readTslotPrefs!\n");

    findDef(fh, "room_cpcty:=");
    readRoomCaps(fh,pi);
    if (debug) printf("End readRoomCaps!\n");

    findDef(fh,"class_limit:=");
    ReadClassLims(fh, pi);
    if (debug) printf("End readClassLims!\n");

    findDef(fh, "kmin:=");
    readKmin(fh, pi);
    if (debug) printf("End readKmin!\n");

    findDef(fh, "kmax:=");
    readKmax(fh, pi);
    if (debug) printf("End readKmax!\n");

    /*************/
    /*************/
    /*************/
    /*************/
    /*************/
    /*************/
    /*************/

    fclose(fh);
    if(debug) printf("End Reading! \n");

    printProblemInstance(pi);

    return 0;
}


