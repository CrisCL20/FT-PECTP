set S; # conjunto de estudiantes
set C; # conjunto de cursos
set A; # conjunto de actividades
set T; # conjunto de bloques disponibles
set R; # conjunto de salones
set Ac {C} within A; # actividades obligatorias del curso C
set Cs {S} within C; # cursos que preinscribe cada estudiante
set Ra {A} within R; # salones que cumplen todas las features que necesita la actividad C
set Ts {S} within T; # bloques por los que el estudiante s marca preferencia
#param CM {M,C} default 0; # 1 si el ramo m in M tiene como requisito la clase c in C, 0 eoc
param rho {R}; # capacidad de cada salon
param sigma_class {C}; # cupo de inscripciones para cada clase
param kmin {S}; # minima cantidad de ramos que cada estudiante puede tomar
param kmax {S}; # maxima cantidad de ramos que cada estudiante puede tomar

var x {a in A, r in R, t in T} binary; # 1 si la clase c se realiza en la sala r en el bloque t, 0 si no.
var yR {a in A, r in R} binary; # 1 si la clase c se realiza en la sala r, 0 si no.
var yT {a in A, t in T} binary; # 1 si la clase c se realiza en el bloque t, 0 si no.
var yA {s in S, a in A} binary; # 1 si el estudiante s asiste a la actividad a
var yC {s in S, c in C} binary; # 1 si el estudiante s queda con el ramo c 
var tau {s in S, a in A, t in T} binary; # 1 si el estudiante s tiene la clase c en el bloque t, 0 si no

#### parametros para MOP
param Mi default 0 ;
param Mf default 0 ;

param cantobj := 2 ; # cantidad de objetivos del problema
param cantejc := 11 ; # cantidad de ejecuciones para la frontera de pareto
set objetivos := {1..cantobj} ; # conjunto de objetivos del problema
set ejecuciones := {1..cantejc} ; # conjunto de ejecuciones para la frontera de pareto
param g default 0 ; # identifica un objetivo en particular
param sigma{ejecuciones,objetivos} ; # ponderadores para la frontera de pareto
param betha{objetivos} default 0 ; # ponderadores de cada objetivo
param MV{objetivos} default 999999999 ; # mejor valor alcanzado por cada objetivo
param PV{objetivos} default 0 ; # peor valor alcanzado por cada objetivo
param M default 99999; #valor grande

var FO{objetivos} >= 0 ; # funciones objetivos del problema

### Funciones objetivo
minimize FO1 : FO[1] ; # para minimizar cada objetivo por separado

subject to
#O1: Minimizar cantidad de horarios no preferidos en que los estudiantes tienen clases
O1 : FO[1] = sum {s in S, t in Ts[s], a in A} tau[s,a,t] ;
O2 : FO[2] = sum {s in S} (card(Cs[s]) - sum{c in C} yC[s,c]); #Minimizar la cantidad de modulos que no inscriben los alumnos.

R1_1 {r in R, a in A} : M*yR[a,r] >= sum{t in T} x[a,r,t]; #Si la clase c se asigna a algun timeslot de la sala r, entonces se activa la variable yR[c,r]

R1_2 {r in R, a in A} : yR[a,r] <= sum{t in T} x[a,r,t]; #Si la clase c se asigna a la sala r, entonces debe asignarse a alguno de los horarios.

R2_1 {t in T, a in A} : M*yT[a,t] >= sum{r in R} x[a,r,t];  #Si la clase c se asigna a alguna sala en el timeslot t, entonces se activa la variable yT[c,t]

R2_2 {t in T, a in A} : yT[a,t] <= sum{r in R} x[a,r,t];  #Si la clase c se asigna al timeslot t, entonces debe asignarse a alguno de las salas

R3 {a in A, r in R diff Ra[a]} : sum{t in T} x[a,r,t] = 0;

R4 {a in A} : sum{t in T} yT[a,t] = 1; # Todas las clases son asignadas a exactamente un horario

R5 {a in A} : sum{r in R} yR[a,r] = 1; # Todas las clases son asignadas a exactamente una sala

R6 {r in R, t in T} : sum{a in A} x[a,r,t] <= 1; # No más de una clase por salón por horario

R7 {a in A} : sum{r in R, t in T} x[a,r,t] = 1; # Cada clase exactamente en un horario en una sala

R8_1 {s in S, c in C diff Cs[s]} : yC[s,c] = 0; # Estudiantes no van a cursos que no solicitaron

R8_2 {s in S, c in C diff Cs[s]} : sum{a in Ac[c]} yA[s,a] = 0; 

R9_1 {s in S} : sum{c in Cs[s]} yC[s,c] <= kmax[s]; # Un estudiante no excede su tope de módulos

R9_2 {s in S} : sum{c in Cs[s]} yC[s,c] >= kmin[s]; # Un estudiante cumple con su mínimo de módulos

# R10_1 {s in S, c in Cs[s]} : card(Ac[c]) + yC[s,c] > sum {a in Ac[c]} yA[s,a] ; # Un estudiante atiende a un módulo si atiende a todas las clases del módulo

# R10_2 {s in S, c in Cs[s]} : M*yC[s,c] <= sum {a in Ac[c]} yA[s,a] ;

R10_1 {s in S, c in Cs[s]} : M*yC[s,c] >= sum{a in  Ac[c]} yA[s,a];

R10_2 {s in S, c in Cs[s]} : card(Ac[c]) * yC[s,c] <= sum {a in Ac[c]} yA[s,a] ; 

R11 {s in S, t in T, a in A} : yA[s,a] + yT[a,t] - 1 <= tau[s,a,t]; # Para cada alumno, en cada instante de tiempo se indica que el alumno s tiene clases en el periodo t siempre y cuando el alumno asista a alguna clase que se dicte en ese bloque de tiempo.

R12_1 {s in S, t in T}: sum{a in A} tau[s,a,t] <=1; #Cada estudiante atiende a lo más una clase por periodo de tiempo.

R12_2 {s in S, a in A}: sum{t in T} tau[s,a,t] <=1; ##

# R13_1 {s in S, c in Cs[s], t in T} : M*tp[s,t] >= sum {a in Ac[c]} tau[s,a,t];

# R13_2 {s in S, c in Cs[s], t in T} : tp[s,t] <= sum {a in Ac[c]} tau[s,a,t];

R14  {a in A, r in R} : sum{s in S} yA[s,a] <= rho[r] + (1-yR[a,r])*M ; # No exceder capacidad de los salones

R15 {c in C} : sum{s in S} yC[s,c] <= sigma_class[c] ; # No exceder el limite de la clase
