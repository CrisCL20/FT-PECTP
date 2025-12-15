set S; # conjunto de estudiantes
set C; # conjunto de clases
set M; # conjunto de ramos
set T; #conjunto de bloques disponibles
set R; # conjunto de salones
set Ms {S}; # ramos que preinscribe cada estudiante
set Rc {C}; # salones que cumplen todas las features que necesita la clase C
set PTs {S} within T; # bloques por los que el estudiante s marca preferencia
set CM {M} within C; # clases del módulo m in M
# param A {R,R} default 0; # matriz con los tiempos de viaje entre salones
# param con {T,T} default 0; # matriz binaria que establece continuidad entre bloques horarios
#param CM {M,C} default 0; # 1 si el ramo m in M tiene como requisito la clase c in C, 0 eoc
param room_cpcty {R}; # capacidad de cada salon
param class_limit {C}; # cupo de inscripciones para cada clase
param kmin {S}; # minima cantidad de ramos que cada estudiante puede tomar
param kmax {S}; # maxima cantidad de ramos que cada estudiante puede tomar

var x {c in C, r in R, t in T} binary; # 1 si la clase c se realiza en la sala r en el bloque t, 0 si no.
var yR {c in C, r in R} binary; # 1 si la clase c se realiza en la sala r, 0 si no.
var yT {c in C, t in T} binary; # 1 si la clase c se realiza en el bloque t, 0 si no.
var yC {s in S, c in C} binary; # 1 si el estudiante s asiste a la clase c
var yM {s in S, m in M} binary; # 1 si el estudiante s queda con el ramo M
var uT {s in S, c in C, t in T} binary; # 1 si el estudiante s tiene la clase c en el bloque t, 0 si no
var h {s in S} >= 0 ; # cantidad de bloques no preferidos en los que quedó el estudiante s

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
param B default 99999; #valor grande

var FO{objetivos} >= 0 ; # funciones objetivos del problema

### Funciones objetivo
minimize FO1 : FO[g] ; # para minimizar cada objetivo por separado
minimize FO2 : sum {i in objetivos} betha[i] * (MV[i] - FO[i])/(MV[i] - PV[i]) ; # para minimizar la funciones objetivos normalizadas

subject to
#O1: Minimizar cantidad de horarios no preferidos en que los estudiantes tienen clases
O1 : FO[1] = sum {s in S} h[s];
O2 : FO[2] = sum {s in S} (card(Ms[s]) - sum{m in Ms[s]} yM[s,m]);

R1_1 {r in R, c in C} : B*yR[c,r] >= sum{t in T} x[c,r,t]; #Si la clase c se asigna a algun timeslot de la sala r, entonces se activa la variable yR[c,r]

R1_2 {r in R, c in C} : yR[c,r] <= sum{t in T} x[c,r,t]; #Si la clase c se asigna a la sala r, entonces debe asignarse a alguno de los horarios.

R2_1 {t in T, c in C} : B*yT[c,t] >= sum{r in R} x[c,r,t];  #Si la clase c se asigna a alguna sala en el timeslot t, entonces se activa la variable yT[c,t]

R2_2 {t in T, c in C} : yT[c,t] <= sum{r in R} x[c,r,t];  #Si la clase c se asigna al timeslot t, entonces debe asignarse a alguno de las salas

R4 {c in C, r in R diff Rc[c]} : sum{t in T} x[c,r,t] = 0;

R6 {c in C} : sum{t in T} yT[c,t] = 1; # Todas las clases son asignadas a exactamente un horario

R7 {c in C} : sum{r in R} yR[c,r] = 1; # Todas las clases son asignadas a exactamente una sala

R8_1 {r in R, t in T} : sum{c in C} x[c,r,t] <= 1; # No más de una clase por salón por horario

R8_2 {c in C} : sum{r in R, t in T} x[c,r,t] = 1; # Cada clase exactamente en un horario en una sala

R14 {s in S, m in M diff Ms[s]} : yM[s,m] = 0; # Estudiantes no van a módulos que no solicitaron

R15 {s in S, m in M diff Ms[s], c in CM[m]} : yC[s,c] = 0; # Estudiante no atiende a clases de un módulo que no solicita

R16 {s in S} : sum{m in Ms[s]} yM[s,m] <= kmax[s]; # Un estudiante no excede su tope de módulos

R17 {s in S} : sum{m in Ms[s]} yM[s,m] >= kmin[s]; # Un estudiante cumple con su mínimo de módulos

R18 {s in S, m in Ms[s]} : yM[s,m] + 1 >= sum{c in CM[m]} yC[s,c];# Un estudiante atiende a un módulo si atiende a todas las clases de este

R19 {s in S, m in Ms[s]} : sum{c in CM[m]} yC[s,c] >= 2*yM[s,m]; # Si un estudiante atiende a un módulo, tiene que atender a todas las clases del módulo
#R19 {s in S, m in Ms[s], c in CM[m]} : 1 - yM[s,m] <= yC[s,c] ; 

#R18 {m in M, s in S, c in C} : yC[s,c]*CM[m,c] <= yM[s,m] ; # Un estudiante atiende a un módulo si atiende a todas las clases del módulo
#
#R19 {m in M, s in S, c in C} : yC[s,c] >= yM[s,m]*CM[m,c] ; # Si un estudiante atiende a un módulo debe atender todas las clases del modulo

R31 {s in S, t in T, c in C} : yC[s,c] + yT[c,t] - 1 <= uT[s,c,t] ;# Para cada alumno, en cada instante de tiempo se indica que el alumno s tiene clases en el periodo t siempre y cuando el alumno asista a alguna clase que se dicte en ese bloque de tiempo.

R32 {s in S, t in T}: sum{c in C} uT[s,c,t] <= 1; #Cada estudiante atiende a lo más una clase por periodo de tiempo.

R33 {s in S, c in C}: sum{t in T} uT[s,c,t] <= 1; ##

R23  {c in C, r in R} : sum{s in S} yC[s,c] <= room_cpcty[r] + (1-yR[c,r])*B ; # No exceder capacidad de los salones

R24 {c in C} : sum{s in S} yC[s,c] <= class_limit[c] ; # No exceder el limite de la clase

R34 {s in S} : h[s] = sum{t in T diff PTs[s], c in C} uT[s,c,t] ; # Total de bloques no preferidos que se agendan