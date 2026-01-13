import os
import xml.etree.ElementTree as ET
import numpy as np
import logging
from pathlib import Path

import ITCinstanemodel as itc

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("parser")

def main(XML_instance, model_dir, output, nm_timeslots, room_cap, class_lim):
    #np.random.seed(42)
    full_path_input = Path(XML_instance)
    full_path_output = os.path.join(model_dir, f"{output}.dat")

    try:
        tree = ET.parse(full_path_input)
        root = tree.getroot()

        ### --- Read rooms
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

        ### --- Read courses
        courses = root[2]
        COURSES: list[itc.Course] = []
        for c in courses:
            course_id = c.items()[0][1]
            course = itc.Course(course_id)
            for config in c:
                cfg = itc.Config(config.items()[0][1])
                for subpart in config:
                    sub = itc.Subpart(subpart.items()[0][1])
                    for activity in subpart:
                        act = itc.Class(activity.items()[0][1], class_lim)
                        sub.add_class(act)
                    if len(sub.classes) > 3:
                        sub.classes = sub.classes[:3]
                    cfg.add_part(sub)
                course.add_config(cfg)
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
        
        logger.info("Finalizado guardado en estructuras...")
    except FileNotFoundError as e:
        logger.error(f"No se pudo encontrar archivo: {e}")

    ###############################################################################
    ###############################################################################
    #################### --- Writing data to instance file --- ####################
    ###############################################################################
    ###############################################################################

    logger.info(f"Comenzando escritura a instance.dat...")
    with open(full_path_output, "w") as dat_file:
        
        cantejc = 11
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

        logger.info("Escribiendo sigmas (parámetro para multiobj)")
        dat_file.write("param sigma\n")
        dat_file.write(": 1   2 :=\n")
        for i, [sigma_a, sigma_b] in enumerate(sigmas):
            dat_file.write(f"{i+1} {sigma_a:.5f} {sigma_b:.5f}\n")
        dat_file.write(";\n")

        # Set of students
        logger.info("Calculando conjunto de estudiantes")
        dat_file.write("set S:=\n")
        for student in STUDENTS:
            dat_file.write(student.id + " ")
        dat_file.write(";\n\n")

        # Set of courses
        logger.info("Calculando conjunto de cursos")
        dat_file.write("set C:=\n")
        n_courses = len(COURSES)
        for course in COURSES:
            dat_file.write(f"{course.id} ")
        dat_file.write(";\n\n")

        # sets of classes per module
        logger.info("Calculando conjunto de actividades...")
        course_activities: dict[itc.Course, list[itc.Class]] = {}
        dat_file.write(f"set A:=\n")
        for course in COURSES:
            course_activities[course] = course.configs[0].subparts[0].classes
            for activity in course_activities[course]:
                dat_file.write(f"{course.id}_{activity.id} ")
        dat_file.write(";\n\n")

        # set of timeslots
        logger.info("Generando conjunto de bloques")
        dat_file.write("set T:=\n")
        # bloques van por dia_idblock donde id block va desde 1-2, 3-4,...,9-10
        # se asume que para todas las semanas es la misma programación
        n_days = 5
        blocks = [f"{i}_{i+1}" for i in range(1,nm_timeslots,2)]
        bloques = []
        for day in range(1,n_days+1):
            for id_block in blocks:
                dat_file.write(f"{day}_{id_block} ")
                bloques.append(f"{day}_{id_block}")
            dat_file.write("\n")
        dat_file.write(";\n\n")

        # set of rooms
        logger.info("Calculando conjunto de salones")
        dat_file.write("set R:=\n")
        for room in ROOMS:
            dat_file.write(f"{room.id} ")
        dat_file.write(";\n\n")
        
        # set of activities
        logger.info("Calculando conjunto de actividades que pertenecen a cada curso...")
        for course in COURSES:
            dat_file.write(f"set Ac[{course.id}]:=\n")
            for activity in course_activities[course]:
                dat_file.write(f"{course.id}_{activity.id} ")
            dat_file.write(";\n")

        dat_file.write("\n")


        # set of modules requested by s in S
        logger.info("Calculando conjunto de cursos que preinscriben los estudiantes...")
        for student in STUDENTS:
            dat_file.write(f"set Cs[{student.id}]:=\n")
            for course in student.courses:
                dat_file.write(f"{course.id} ")
            dat_file.write(";\n")
        dat_file.write("\n")

        # set of adequate rooms for class c in C
        logger.info("Calculando conjunto de salones que son adecuados para cada clase...")
        for course, activities in course_activities.items():
            for i in range(len(activities)):
                dat_file.write(f"set Ra[{course.id}_{activities[i].id}]:=\n")
                nm_rooms = np.random.randint(2, len(ROOMS))
                rooms_for_act = sorted(np.random.choice(ROOMS, nm_rooms, replace=False), key=lambda x: x.id)
                for room in rooms_for_act:
                    dat_file.write(f"{room.id} ")
                dat_file.write(";\n")
        dat_file.write("\n\n")

        logger.info("Calculando preferencias de bloques por estudiante...")
        # Por defecto, un estudiante puede ir todos los días
        # Pero dentro de estos días, prefiere ir a ciertos bloques
        
        preferred_timesets: dict[itc.Student, list] = {}
        for student in STUDENTS:
            nm_preferred_slots = np.random.randint(0, max(0.7 * len(student.courses),3))
            preferred_timesets[student] = np.random.choice(bloques, nm_preferred_slots, replace=False)
        # Write PTs sets
        for s in STUDENTS:
            dat_file.write(f"set Ts[{s.id}]:= \n")
            for pref_ts in preferred_timesets[s]:
                dat_file.write(f"{pref_ts} ")
            dat_file.write(" ; \n")
        dat_file.write("\n")

        # room capacity
        logger.info("Calculando capacidad de salones")
        dat_file.write("param rho:=\n")
        for room in ROOMS:
            dat_file.write(f"{room.id} {room_cap}\n")
        dat_file.write(";\n\n")

        logger.info("Calculando limite de clases")
        dat_file.write("param sigma_class:=\n")
        for c in COURSES:
            dat_file.write(f"{c.id} {class_lim}\n")
        dat_file.write(";\n\n")

        # Min and max courses per student

        logger.info("Calculando minimo de cursos que un estudiante debe tener inscritos")
        w_min = .5 #De los ramos que preinscribe, como mínimo se deben programar w_min% 
        dat_file.write("param kmin:=\n")
        for student in STUDENTS:
            n_preinscriptions = len(student.courses)
            min_inscriptions = int(np.floor(w_min * n_preinscriptions))
            dat_file.write(f"{student.id} {min_inscriptions}\n")
        dat_file.write(";\n\n")

        logger.info("Calculando máximo de clases que un estudiante s puede inscribir")
        dat_file.write("param kmax:=\n")
        for student in STUDENTS:
            n_preinsc = len(student.courses)
            dat_file.write(f"{student.id} {n_preinsc}\n")
        dat_file.write(";\n\n")
        
        logger.info("Instancia creada con éxito!")
    

if __name__ == "__main__":
    
    import argparse
    import threading

    parser = argparse.ArgumentParser(
        prog="ITC instance parser"
    )

    parser.add_argument("XML_instance")
    parser.add_argument("model_dir")
    parser.add_argument("output")
    parser.add_argument("nm_timeslots", type=int)
    parser.add_argument("room_cap", type=int)
    parser.add_argument("class_lim", type=int)

    args = parser.parse_args()

    thread = threading.Thread(name="ITC instance parser", target=main,
                              args=(args.XML_instance, args.model_dir, args.output, 
                                    args.nm_timeslots, args.room_cap, args.class_lim))
    
    thread.start()
    thread.join()

    logger.info(f"Creada instancia para {args.XML_instance}")
    