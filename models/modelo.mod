## En este modelo aún no se agregan las modalidades

set S; # cjto de estudiantes
set C; # cjo de clases
set M; # cjto de modulos
set L; # bloques de tiempo disponibles
set T; # cjto de horarios con cada elemento un cjto de bloques
set L_in_T {T} within L; # especificación de cada uno de los horarios en T
set R; # cjuto de salones
set Ms {S} within M; #modulos pedidos por s in S
#set Mobs {S} within M; # modulos obligatorios
set Mels {S} within M; #modulos electivos por s in S
set Sm {M} within S; #estudiantes que quieren ir al módulo m in M
set RrU {R} within L; #bloques cuando r in R no está disp.
set Rc {C} within R; #cjto de salones adecuados para la clase c
set Tc {C} within T; # cjto de horarios adecuados para c
#set RG {G within C} := union {c in G} Rc[c]; # G es un subcjto de clases, RG[G] son los salones 
                                             # que son adecuados para cada clase c in G
set RrC {r in R} := {c in C: r in Rc[c]}; # clases tales que r sea adecuado para esas clases (que tenga las fetures requerida por la clase c)
set Ol {l in L} := union {t in T: l in L_in_T[t]} {t}; # Ol
set Fm {M}; # configuraciones para cada módulo k in K
set P {m in M, f in Fm[m]}; # subpartes de cada configuración
set Cmfp {m in M, f in Fm[m], p in P[m,f]}; # clases en la subparte p de la configuracion f del módulo k
set Cs {s in S} within C := union {m in Ms[s], f in Fm[m], p in P[m,f]} Cmfp[m,f,p]; # conjunto de clases a las que el estudiante s podría atender
set C_CS {s in S} := {c1 in Cs[s], c2 in Cs[s] : c1 != c2}; # pares únicos de clases en G within C

######### params #########
param A {r1 in R, r2 in R: r1 != r2} default 0; # tiempos entre cada salon
param room_cpcty {R}; # limite de los salones
param class_limit {C}; # limite de las clases
# Esta aún no #param pi_s {S} default 0; # preferencia de cada estudiante s in S (se puede generar aleatoriamente)
param D0 {R,T} default 0; # 1 si r no está disponible en algún bloque en el horario t
#param D1 {t1 in T,t2 in T} default 1; # 1 si t1 choca con t2
#param D2 {r1 in R,r2 in R,t1 in T,t2 in T} default 1; 
#param room_has_feats {R,F} binary;
#param pre_enrollment {S,C} binary;
param kmax {S}; # máxima cantidad de clases que se pueden tomar
/* Conjuntos, Parametros y Variables para la normalización */

### Variables a conservar: 
# - h_{s,c_1, c_2}: indica si hay problemas de programación al asignar a s a c1 y c2
# - x_{c,r,t}: indica si el curso c se asigna a la sala r en el bloque t
# - a_{s,c}: indica si s es asignado al curso c --- por ahora los cursos tienen una estructura más simple, por lo que para 2019 tendremos que usar la variable n_{s,k} para electivos

var x{c in C, r in R, t in T} binary;
var yR{c in C, r in R} binary;
var yT{c in C, t in T} binary;
var g_m{m in M} binary;
var q_mf{m in M, f in Fm[m]} binary;
var w_mfp{m in M, f in Fm[m], p in P[m,f]} binary;
var a_smfpc{s in S, m in M, f in Fm[m], p in P[m,f], c in Cmfp[m,f,p]} binary;
var alpha{s in S, c in C} binary;
var b_smfp{s in S, m in M, f in Fm[m], p in P[m,f]} binary;
var m_smf{s in S, m in M, f in Fm[m]} binary;
var n{s in S, m in M} binary;
var beta{s in S, c in C, t in T} binary;
var gamma_scrt{s in S, c in C, r in R, t in T} binary;
var h{s in S, c1 in C, c2 in C} binary;

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
var FO{objetivos} >= 0 ; # funciones objetivos del problema

### Funciones objetivo
minimize FO1 : FO[g] ; # para minimizar cada objetivo por separado
minimize FO2 : sum {i in objetivos} betha[i] * (MV[i] - FO[i])/(MV[i] - PV[i]) ; # para minimizar la funciones objetivos normalizadas

subject to
# O1: Minimizar el total de colisiones en la programación de cada estudiante 
O1 : FO[1] = sum{s in S, c1 in C, c2 in C} h[s,c1,c2] ;
#R2: Maximizar cursos electivos (para 2007 se podría definir un porcentaje de cursos que son electivos por cada estudiante)
O2 : FO[2] = -1 * sum {s in S, m in Mels[s]} n[s,m]; # -1 porque en las FO estamos minimizando (queremos maximizar aquí)

R1 {r in R, c in C} : yR[c,r] = sum{t in T} x[c,r,t];
### Como no hay modalidad hibrida, podemos hacerlo con 1 única restricción
R2 {t in T, c in C} : yT[c,t] = sum{r in R} x[c,r,t];
### Clases solo pueden ser asignadas a salones y horarios
### compatibles
R3 {c in C, t in T diff Tc[c]} : sum{r in R} x[c,r,t] = 0;
R4 {c in C, r in R diff Rc[c]} : sum{t in T} x[c,r,t] = 0;

### Las clases no deberían suceder en un salón cuando este
### no se encuentra disponible
R5 {c in C} : sum{t in T, r in R} D0[r,t] * x[c,r,t] = 0;

### Clases son asignadas a lo más en un horario
R6 {c in C} : sum{t in T} yT[c,t] <= 1;

### Clases se dictan en a lo más 1 sala
R7 {c in C} : sum{r in R} yR[c,r] <= 1;

### No más de una clase por salón
R8 {r in R, l in L} : sum{c in RrC[r], t in Ol[l]} x[c,r,t] <= 1;

### Un módulo se ofrece si se ofrece al menos una configuración
R9 {m in M} : g_m[m] * card(Fm[m]) >= sum{f in Fm[m]} q_mf[m,f] ;
R10 {m in M} : g_m[m] <= sum{f in Fm[m]} q_mf[m,f] ; 

### Una configuración se ofrece si se ofrecen todas sus subpartes
R11 {m in M, f in Fm[m]} : q_mf[m,f] * card(P[m,f]) = sum{p in P[m,f]} w_mfp[m,f,p];

### Una subparte se ofrece si al menos una clase en la subparte se ofrece
R12 {m in M, f in Fm[m], p in P[m,f]} : 
    w_mfp[m,f,p] * card(Cmfp[m,f,p]) * card(R) * card(T) >=
                sum {c in Cmfp[m,f,p], r in R, t in T} x[c,r,t];
R13 {m in M, f in Fm[m], p in P[m,f]} :
    w_mfp[m,f,p] <= sum{c in Cmfp[m,f,p], r in R, t in T} x[c,r,t];

### Estudiantes no van a módulos que no solicitaron
R14 {s in S, m in M diff Ms[s]} : n[s,m] <= 0;

### Estudiantes no van a un módulo que no se ofrece
R15 {s in S, m in M} : n[s,m] <= g_m[m];

### Un estudiante debe atender a todos sus cursos obligatorios
### por ahora Mobs = Mels
#R16 {s in S, m in Mels[s]} : n[s,m] = 1;

### Un estudiante no excede su tope de módulos
R17 {s in S} : sum{m in M} n[s,m] <= kmax[s];

### Un estudiante no atiende a una clase que no es ofrecida
R18 {c in C, s in S} : alpha[s,c] <= sum{t in T, r in R} x[c,r,t];

### Un estudiante atiende a un módulo si atiende a una configuración del módulo
R19 {m in M, s in S} : sum{f in Fm[m]} m_smf[s,m,f] = n[s,m] ;

### Cada estudiante asigna una configuración si van a una clase de cada
### subparte
R20 {s in S, m in M, f in Fm[m]} : sum{p in P[m,f]} b_smfp[s,m,f,p] = card(P[m,f]) * m_smf[s,m,f];

### Cada estudiante tiene a lo más una clase de una subparte
R21 {s in S, m in M, f in Fm[m], p in P[m,f]} : 
    sum {c in Cmfp[m,f,p]} a_smfpc[s,m,f,p,c] = b_smfp[s,m,f,p] ;

### Esta podría ser redundante
R22 {s in S, m in M, f in Fm[m], p in P[m,f], c in Cmfp[m,f,p]} :
    a_smfpc[s,m,f,p,c] = alpha[s,c] ;

### No exceder capacidad de los salones
R23 {m in M, f in Fm[m], p in P[m,f], c in Cmfp[m,f,p]} :
    sum{s in Sm[m]} alpha[s,c] <= sum{r in Rc[c]} room_cpcty[r] * yR[c,r] ;

### No exceder el limite de la clase
R24 {m in M, f in Fm[m], p in P[m,f], c in Cmfp[m,f,p]} :
     sum{s in Sm[m]} alpha[s,c] <= class_limit[c] ;

### Clases padre-hijo <- Ver que onda
#R25 {s in S}

### detectar topes en clases 
R25 {s in S, c in C, t in T} : alpha[s,c] * yT[c,t] = beta[s,c,t] ;
#R26 {s in S, (c1,c2) in C_CS[s], t1 in Tc[c1], t2 in Tc[c2]} :
#   D1[t1, t2] * (beta[s,c1,t1] + beta[s,c2,t2]) <= 1 + h[s,c1,c2] ;

### detectar si un estudiante tiene el tiempo suficiente para ir de una sala a otra
R27 {s in S, c in C, t in T, r in R} : beta[s,c,t] * yR[c,r] = gamma_scrt[s,c,r,t] ;
#R28 {s in S, (c1,c2) in C_CS[s], t1 in Tc[c1], t2 in Tc[c2], r1 in Rc[c1], r2 in Rc[c2]} : 
#    D2[r1,r2,t1,t2] * (gamma_scrt[s,c1,r1,t1] + gamma_scrt[s,c2,r2,t2]) <= 1 + h[s,c1,c2] ;
