import sys
import os
import xml.etree.ElementTree as ET
from typing import Any
import numpy as np

import ITCinstanemodel as itc


def get_preferred_blocks(n_modules, blocks, rng):
    preferred_timeslots = []
    while len(preferred_timeslots) <= 2 * n_modules:
        choice_rng = rng.choice(blocks)
        if choice_rng not in preferred_timeslots:
             print(f"Prefiere {choice_rng}")
             preferred_timeslots.append(choice_rng)
    preferred_timeslots = sorted(preferred_timeslots, key=lambda x: int(x[0]))
    return preferred_timeslots

def main(instance_file, model_dir, ofile, rooms_cap, classes_limit):
    #np.random.seed(42)
    full_path_input = os.path.join("instances", f"{instance_file}.xml")
    full_path_output = os.path.join(model_dir, f"{ofile}.dat")

    if os.path.exists(full_path_input):
        tree = ET.parse(full_path_input)
        root = tree.getroot()

        opt_tag = root[0]
        time, room_, dist, student_ = opt_tag.items()
        
        rooms = root[1]
        ROOMS: list[itc.Room] = []
        for r in rooms: 
            r_id, cpcty = r.items()
            room = itc.Room(r_id[1], int(cpcty[1]))
            if(len(r) > 0):
                for trav_or_unv in r:
                    if trav_or_unv.tag == "travel":
                        dest, ttime = trav_or_unv.items()
                        room.add_travel_time(int(dest[1]), int(ttime[1]))
                    elif trav_or_unv.tag == "unavailable":
                        days, start, length, weeks = trav_or_unv.items()
                        formatted_days = []
                        formatted_weeks = []
                        for i,d in enumerate(days[1]):
                            if d == '1':
                                formatted_days.append(i+1)
                        for i, w in enumerate(weeks[1]):
                            if w == '1':
                                formatted_weeks.append(i+1)
                        room.add_unavailability(formatted_days, int(start[1]), int(length[1]), formatted_weeks)
            ROOMS.append(room)

        courses = root[2]
        COURSES: list[itc.Course] = []
        for c in courses:
            course_id = c.items()[0][1]
            course = itc.Course(course_id)
            COURSES.append(course)
        
        students = root[4]
        STUDENTS: list[itc.Student] = []
        for student in students:
            student_id = student.items()[0][1]
            new_student = itc.Student(student_id)
            for course in student:
                course_id = course.items()[0][1]
                for course in COURSES:
                    if course.id == course_id:
                        new_student.add_course(course)
                        break
            STUDENTS.append(new_student)
        
        print("Finalizado guardado en estructuras...")
        ###############################################################################
        ###############################################################################
        #################### --- Writing data to instance file --- ####################
        ###############################################################################
        ###############################################################################

        print(f"Comenzando escritura a instance.dat...")
        with open(full_path_output, "w") as dat_file:
            
            cantejc = 22
            sigmas = np.zeros(shape=(cantejc, 2),dtype=np.float64)

            sigmas[0,0] = .00001
            sigmas[0,1] = .99999
            
            half_point = cantejc // 2
            
            weights = (1+np.arange(half_point-1)) / (cantejc - 1)
            sigmas[1:half_point,0] = weights
            sigmas[1:half_point,1] = 1 - weights

            sigmas[half_point,:] = [.5,.5]
            
            sigmas[half_point+1:-1,0] = 1 - weights
            sigmas[half_point+1:-1,1] = weights
            
            sigmas[-1,:] = [0.99999,0.00001]

            print("Escribiendo sigmas (parámetro para multiobj)")
            dat_file.write("param sigma\n")
            dat_file.write(": 1   2 :=\n")
            for i, [sigma_a, sigma_b] in enumerate(sigmas):
                dat_file.write(f"{i+1} {sigma_a:.5f} {sigma_b:.5f}\n")
            dat_file.write(";\n")

            # Set of students
            print("Calculando conjunto de estudiantes")
            dat_file.write("set S:=\n")
            for student in STUDENTS:
                dat_file.write(student.id + " ")
            dat_file.write(";\n\n")

            # Set of classes
            print("Calculando conjunto de clases")
            dat_file.write("set C:=\n")
            n_courses = len(COURSES)
            n_classes = 2 * n_courses ## 2 clases de cátedra por ramo
            for i in range(n_classes):
                dat_file.write(f"{i+1} ")
            dat_file.write(";\n\n")

            # Set of modules
            print("Calculando conjunto de cursos (ramos)")
            dat_file.write("set M:=\n")
            for module in COURSES:
                dat_file.write(module.id+" ")
            dat_file.write(";\n\n")

            # set of timeslots
            print("Generando conjunto de bloques")
            dat_file.write("set T:=\n")
            # bloques van por dia_idblock donde id block va desde 1-2, 3-4,...,9-10
            # se asume que para todas las semanas es la misma programación
            n_days = 5
            n_timeblocks = 20
            blocks = [f"{i}_{i+1}" for i in range(1,n_timeblocks,2)]
            bloques = []
            for day in range(1,n_days+1):
                for id_block in blocks:
                    dat_file.write(f"{day}_{id_block} ")
                    bloques.append(f"{day}_{id_block}")
                dat_file.write("\n")
            dat_file.write(";\n\n")

            # set of rooms
            print("Calculando conjunto de salones")
            dat_file.write("set R:=\n")
            for room in ROOMS:
                dat_file.write(f"{room.id} ")
            dat_file.write(";\n\n")

            # sets of classes per module
            print("Calculando conjunto de clases que pertenecen a cada módulo.")
            for m in COURSES:
                dat_file.write(f"set CM[{m.id}] :=\n")
                c1 = 2*int(m.id) - 1
                c2 = 2*int(m.id)
                dat_file.write(f"{c1} {c2} ;\n")
            dat_file.write("\n")


            # set of modules requested by s in S
            print("Calculando conjunto de cursos que preinscribe s in S")
            for student in STUDENTS:
                dat_file.write(f"set Ms[{student.id}]:=\n")
                for course in student.courses:
                    dat_file.write(f"{course.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of adequate rooms for class c in C
            print("Conjunto de salones que son adecuados para cada clase")
            for i in range(n_classes):
                dat_file.write(f"set Rc[{i+1}]:=\n")
                for room in ROOMS:
                    if (i+1+int(room.id)) % 2:
                        dat_file.write(f"{room.id} ")
                dat_file.write(";\n")
            dat_file.write("\n\n")

            print("Calculando preferencias de bloques por estudiante")
            # Por defecto, un estudiante puede ir todos los días
            # Pero dentro de estos días, prefiere ir a ciertos bloques
            rng = np.random.default_rng(seed=755)
            preferred_timesets: dict[itc.Student, list] = {}
            for student in STUDENTS:
                preferred_timesets[student] = []
                n_preferred_modules = len(student.courses)
                print(f"Calculando estudiante {student.id}")
                preferred_timeslots = get_preferred_blocks(n_preferred_modules, bloques, rng)
                
                for pref in preferred_timeslots:
                    preferred_timesets[student].append(pref)
            # Write PTs sets
            for s in STUDENTS:
                dat_file.write(f"set PTs[{s.id}] := \n")
                for pref_ts in preferred_timesets[s]:
                    dat_file.write(f"{pref_ts} ")
                dat_file.write(" ; \n")
            dat_file.write("\n")

            # class per course matrix
            # print("Calculando matriz CM")
            # dat_file.write("param CM:=\n")

            # classes: list[tuple[int,int]] = []
            # for i in range(0,n_classes,2):
            #     classes.append((i+1,i+2))

            # next_classes = classes
            # for course in COURSES:
            #     c1, c2 = next_classes[0]
            #     dat_file.write(f"{course.id} {c1} 1 {course.id} {c2} 1\n")
            #     next_classes = next_classes[1:]
            # dat_file.write(";\n\n")

            # room capacity
            ### NOTE: capacidad se debería obtener desde argv[2]
            print("Calculando capacidad de salones")
            dat_file.write("param room_cpcty :=\n")
            for room in ROOMS:
                dat_file.write(f"{room.id} {rooms_cap}\n")
            dat_file.write(";\n\n")

            ### NOTE: lo mismo pero con argv[3]
            print("Calculando limite de clases")
            dat_file.write("param class_limit:=\n")
            for i in range(n_classes):
                dat_file.write(f"{i+1} {classes_limit}\n")
            dat_file.write(";\n\n")

            # Min and max courses per student

            print("Calculando minimo de cursos que un estudiante debe tener inscritos")
            w_min = .5 #De los ramos que preinscribe, como mínimo se deben programar w_min% 
            dat_file.write("param kmin :=\n")
            for student in STUDENTS:
                n_preinscriptions = len(student.courses)
                min_inscriptions = int(np.floor(w_min * n_preinscriptions))
                dat_file.write(f"{student.id} {min_inscriptions}\n")
            dat_file.write(";\n\n")

            print("Calculando máximo de clases que un estudiante s puede inscribir")
            dat_file.write("param kmax :=\n")
            for student in STUDENTS:
                n_preinsc = len(student.courses)
                dat_file.write(f"{student.id} {n_preinsc}\n")
            dat_file.write(";\n\n")
            
            print("Instancia creada con éxito!")
    else:
        print("No existe el archivo")

if __name__ == "__main__":
    
    ifile = sys.argv[1]
    model_dir = sys.argv[2]
    ofile = sys.argv[3]
    rooms_cap = sys.argv[4]
    classes_limit = sys.argv[5]
    main(ifile, model_dir, ofile, rooms_cap, classes_limit)
    print(f"Creada instancia para {ifile}")
    