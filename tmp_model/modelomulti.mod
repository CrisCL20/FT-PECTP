set S; # conjunto de estudiantes
set C; # conjunto de clases
set M; # conjunto de ramos
set T; #conjunto de bloques disponibles
set R; # conjunto de salones
set Ms {S}; # ramos que preinscribe cada estudiante
set Rc {C}; # salones que cumplen todas las features que necesita la clase C
param A {R,R} default 0; # matriz con los tiempos de viaje entre salones
param con {T,T} default 0; # matriz binaria que establece continuidad entre bloques horarios
param CM {M,C} default 0; # 1 si el ramo m in M tiene como requisito la clase c in C, 0 eoc
param room_cpcty {R}; # capacidad de cada salon
param class_limit {C}; # cupo de inscripciones para cada clase
param kmin {S}; # minima cantidad de ramos que cada estudiante puede tomar
param kmax {S}; # maxima cantidad de ramos que cada estudiante puede tomar

var x {c in C, r in R, t in T} binary; # 1 si la clase c se realiza en la sala r en el bloque t, 0 si no.
var yR {c in C, r in R} binary; # 1 si la clase c se realiza en la sala r, 0 si no.
var yT {c in C, t in T} binary; # 1 si la clase c se realiza en el bloque t, 0 si no.
var yC {s in S, c in C} binary; # 1 si el estudiante s asiste a la clase c
var yM {s in S, m in M} binary; # 1 si el estudiante s queda con el ramo M
var z {c1 in C, c2 in C} binary; # 1 si al menos un alumno atiende las clases c1 y c2, 0 si no
var u {s in S, t1 in T, t2 in T} binary;  # 1 si el estudiante s tiene clases en el bloque t1 y en el bloque t2, 0 si no
var uT {s in S, t in T} binary; # 1 si el estudiante s tiene clases en el bloque t, 0 si no
var uCT {s in S, c in C, t in T} binary; # 1 si el estudiante s atiende la clase c en el bloque t, 0 si no

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
#O1: Minimizar tiempos de viajes
O1 : FO[1] = sum{s in S, t1 in T, t2 in T} u[s,t1,t2] ;
#R2: Minimizar la diferencia entre cursos pre-inscritos y efectivamente inscritos.
O2 : FO[2] = sum {s in S} (card(Ms[s]) - sum{m in M} yM[s,m]); # -1 porque en las FO estamos minimizando (queremos maximizar aquí)

R1_1 {r in R, c in C} : B*yR[c,r] >= sum{t in T} x[c,r,t]; #Si la clase c se asigna a algun timeslot de la sala r, entonces se activa la variable yR[c,r]

R1_2 {r in R, c in C} : yR[c,r] <= sum{t in T} x[c,r,t]; #Si la clase c se asigna a la sala r, entonces debe asignarse a alguno de los horarios.

R2_1 {t in T, c in C} : B*yT[c,t] >= sum{r in R} x[c,r,t];  #Si la clase c se asigna a alguna sala en el timeslot t, entonces se activa la variable yT[c,t]

R2_2 {t in T, c in C} : yT[c,t] <= sum{r in R} x[c,r,t];  #Si la clase c se asigna al timeslot t, entonces debe asignarse a alguno de las salas

R4 {c in C, r in R diff Rc[c]} : sum{t in T} x[c,r,t] = 0;

R6 {c in C} : sum{t in T} yT[c,t] = 1; # Todas las clases son asignadas a exactamente un horario

R7 {c in C} : sum{r in R} yR[c,r] = 1; # Todas las clases son asignadas a exactamente una sala

R8_1 {r in R, t in T} : sum{c in Rc[r]} x[c,r,t] <= 1; # No más de una clase por salón por horario

R8_2 {c in C} : sum{r in R, t in T} x[c,r,t] = 1; # Cada clase exactamente en un horario en una sala

### Una subparte se ofrece si al menos una clase en la subparte se ofrece
#R12 {m in M, f in Fm[m], p in P[m,f]} :
#    w_mfp[m,f,p] * card(Cmfp[m,f,p]) * card(R) * card(T) >=
#                sum {c in Cmfp[m,f,p], r in R, t in T} x[c,r,t];

#R13 {m in M, f in Fm[m], p in P[m,f]} :
#    w_mfp[m,f,p] <= sum{c in Cmfp[m,f,p], r in R, t in T} x[c,r,t];

R14 {s in S, m in M diff Ms[s]} : yM[s,m] = 0; # Estudiantes no van a módulos que no solicitaron

### Estudiantes no van a un módulo que no se ofrece
#R15 {s in S, m in M} : n[s,m] <= g_m[m];

R16 {s in S} : sum{m in Ms[s]} yM[s,m] <= kmax[s]; # Un estudiante no excede su tope de módulos

R17 {s in S} : sum{m in Ms[s]} yM[s,m] >= kmin[s]; # Un estudiante cumple con su mínimo de módulos

R18 {m in M, s in S, c in C} : yC[s,c]*CM[m,c] <= yM[s,m] ; # Un estudiante atiende a un módulo si atiende a todas las clases del módulo

R19 {m in M, s in S, c in C} : yC[s,c] >= yM[s,m]*CM[m,c] ; # Si un estudiante atiende a un módulo debe atender todas las clases del modulo

R23  {c in C, r in R} : sum{s in S} yC[s,c] <= room_cpcty[r] + (1-yR[c,r])*B ; # No exceder capacidad de los salones

R24 {c in C} : sum{s in S} yC[s,c] <= class_limit[c] ; # No exceder el limite de la clase

R30 {s in S, t1 in T, t2 in T} :  uT[s,t1] + uT[s, t2] - 1 <= u[s,t1,t2] + B*(1-con[t1,t2]) ;

R31 {s in S, t in T, c in C} : yC[s,c] + yT[c,t] -1 <= uT[s,t] ;# Para cada alumno, en cada instante de tiempo se indica que el alumno s tiene clases en el periodo t siempre y cuando el alumno asista a alguna clase que se dicte en ese bloque de tiempo.

R32 {s in S, c in C, t in T}: yC[s,c] + yT[c,t] -1 <= uCT[s,c,t] ;

R33 {s in S, t in T} : sum{c in C} uCT[s,c,t] <= 1 ;# Para cada alumno, en cada instante de tiempo se indica que el alumno s tiene clases en el periodo t siempre y cuando el alumno asista a alguna clase que se dicte en ese bloque de tiempo.

### detectar topes en clases
#R25 {s in S, c in C, t in T} : alpha[s,c] * yT[c,t] = beta[s,c,t] ;
#R26 {s in S, (c1,c2) in C_CS[s], t1 in Tc[c1], t2 in Tc[c2]} :
#   D1[t1, t2] * (beta[s,c1,t1] + beta[s,c2,t2]) <= 1 + h[s,c1,c2] ;

### detectar si un estudiante tiene el tiempo suficiente para ir de una sala a otra
#R27 {s in S, c in C, t in T, r in R} : beta[s,c,t] * yR[c,r] = gamma_scrt[s,c,r,t] ;
#R28 {s in S, (c1,c2) in C_CS[s], t1 in Tc[c1], t2 in Tc[c2], r1 in Rc[c1], r2 in Rc[c2]} :
#    D2[r1,r2,t1,t2] * (gamma_scrt[s,c1,r1,t1] + gamma_scrt[s,c2,r2,t2]) <= 1 + h[s,c1,c2] ;
