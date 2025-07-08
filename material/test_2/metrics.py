import numpy as np
import pandas as pd
import os

def  dominates(a: np.array, b: np.array) -> int:
    """
    Function to determine dominance between a and b
    Args:
        a: point
        b: point

    Returns:
    @ 1 If a dominates b
    @ 0 If b dominates a
    @ -1 If they are nondominated
    """

    n = len(a) #Cantidad de funciones objetivo

    #Si los puntos son iguales
    if (a==b).all():
        return -1

    #Si A domina a B
    if sum(a<=b) == n:
        return 1
    #Si B domina a A
    elif sum(b<=a) == n:
        return 0

    #Si no, no son comparables
    return -1


def two_set_coverage(A, B):
    count = 0
    for b in B:
        for a in A:
            #print(a,b)
            if dominates(a,b) == 1:
                count += 1
                break
    return count / len(B)


def filter_dominated(pf):
    non_dominated = []
    for i, sol1 in enumerate(pf):
        is_dominated = False
        for j, sol2 in enumerate(pf):
            if i != j and dominates(sol2, sol1) == 1:
                is_dominated = True
                break
        if not is_dominated:
            non_dominated.append(sol1)

    return non_dominated


def load_file(my_file):
    f = open(my_file)
    A = []
    while True:
        line = f.readline()
        if line == "":
            return A
        vector = line.split()
        if len(vector) > 1:
            A.append(np.array(list(map(float, vector))))


if __name__ == '__main__':
    instances = ["instancia_juguete", "instancia_grande"]
    AB = []
    BA = []

    for e in instances:
        A = load_file(f"{e}_nsga.dat")
        filtred = filter_dominated(A)

        name, ext = os.path.splitext(f"{e}_nsga.dat")
        output_path = name + "_filtrado" + ext
        with open(output_path, 'w') as f:
            for sol in filtred:
                f.write(" ".join(map(str, sol)) + "\n")
        #print(A)
        #print(A[0])
        #filter_dominated(A)
        #exit()


        #B = load_file("Solver_bomctop_2_"+str(e[0])+"_"+str(e[1])+".dat")
        
        #HV
        #compute maximum values
        #for seed in seeds:
        #    pf=load_file("Results/pf_bomctop_2_"+str(e[0])+"_"+str(e[1])+".dat_"+str(seed)+".out")
        #    print("Results/pf_bomctop_2_"+str(e[0])+"_"+str(e[1])+".dat_"+str(seed)+".out")
        #    filter_dominated(pf)

    # df = pd.DataFrame()
    # df["I_SC(A,B)"] = AB
    # df["I_SC(B,A)"] = BA
    # df.style.to_latex("table_tsc.tex")
