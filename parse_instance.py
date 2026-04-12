import os
import xml.etree.ElementTree as ET
import numpy as np
from typing import Literal, Callable
import logging
from pathlib import Path

import ITCinstanemodel as itc

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("parser")

def write_sigmas(dat_file):
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

def parse_itc(XML_instance, model_dir, output, nm_timeslots, room_cap, class_lim):
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
        
        write_sigmas(dat_file)

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

def parse_sori(nm_students, nm_courses, nm_rooms, nm_timeslots, rho, sigma_class, output_path):
    out_file = Path(f"{output_path}")
    students = np.arange(1, nm_students+1, dtype=int)
    courses = np.arange(1, nm_courses+1, dtype=int)
    rooms = np.arange(1, nm_rooms+1, dtype=int)

    logger.info("Generating sets and parameters...")

    # Generate timeslots
    n_days = 5
    blocks = [f"{i}_{i+1}" for i in range(1,nm_timeslots,2)]
    T = []
    for day in range(1,n_days+1):
        for id_block in blocks:
            T.append(f"{day}_{id_block}")
    
    T = np.array(T)

    # Generate activities for courses and final activities set
    n_activities_per_course = np.random.randint(2,4, nm_courses, dtype=int)
    n_activities_per_course = { c : n_activities_per_course[idx] for idx, c in enumerate(courses)}
    Ac = { int(c) : [f"{c}_{n_act+1}" for n_act in np.arange(n_acts)] for c, n_acts in n_activities_per_course.items()}

    activities = list(Ac.values())
    activities = [x for xs in activities for x in xs]

    # Generate room availability for activities
    n_rooms_per_activity = np.random.randint(1, (nm_rooms+1) // 1.5, len(activities))
    n_rooms_per_activity = {a : n_rooms_per_activity[idx] for idx, a in enumerate(activities)}

    Ra = { a : sorted(np.random.choice(rooms, n_rooms, replace=False).tolist()) for a, n_rooms in n_rooms_per_activity.items()}
    
    # Generate student course demand

    n_courses_per_student = np.random.randint(3, 7, nm_students)
    n_courses_per_student = { e : n_courses_per_student[idx] for idx, e in enumerate(students)}

    Cs = { int(e) : sorted(np.random.choice(courses, n_courses, replace=False).tolist()) for e, n_courses in n_courses_per_student.items()}

    # Generate student timeslot demand
    n_timeslots_per_student = np.random.randint(3, T.size // 3, nm_students)
    n_timeslots_per_student = { e: n_timeslots_per_student[idx] for idx, e in enumerate(students)}

    Ts = { int(e) : T[sorted(np.random.choice(np.arange(T.size), n_tslots, replace=False).tolist())].tolist() for e, n_tslots in n_timeslots_per_student.items()}

    # kmin and kmax
    wmin = .5
    kmin = { int(e) : int(np.floor(wmin * n_courses_per_student[e])) for e in students}
    kmax = { int(e) : int(n_courses_per_student[e]) for e in students}

    logger.info("DONE")
    with open(out_file, "w") as dat_file:
        logger.info(f"\tWritting to {output_path}")

        write_sigmas(dat_file)

        logger.info("\tWritting set of students...")
        dat_file.write("set S:=\n")
        for e in students:
            dat_file.write(f"{int(e)} ")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting set of courses...")
        dat_file.write("set C:=\n")
        for c in courses:
            dat_file.write(f"{int(c)} ")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting set of activities...")
        dat_file.write("set A:=\n")
        for act in activities:
            dat_file.write(f"{act} ")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting set of Timeslots...")
        dat_file.write("set T:=\n")
        for i, ts in enumerate(T.tolist()):
            dat_file.write(f"{ts} ")
            if (i+1) % (5 * (nm_timeslots / 10)) == 0:
                dat_file.write("\n")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting set of rooms...")
        dat_file.write("set R:=\n")
        for r in rooms:
            dat_file.write(f"{int(r)} ")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting set of course activities...")
        for c, acts in Ac.items():
            dat_file.write(f"set Ac[{c}]:=\n")
            for a in acts:
                dat_file.write(f"{a} ")
            dat_file.write(";\n")
        dat_file.write("\n")
        logger.info("DONE")
        
        logger.info("\tWritting set of student course preferenes...")
        for e, courses in Cs.items():
            dat_file.write(f"set Cs[{e}]:=\n")
            for c in courses:
                dat_file.write(f"{c} ")
            dat_file.write(";\n")
        dat_file.write("\n")
        logger.info("DONE")

        logger.info("\tWritting set of room availbalities...")
        for a, rooms in Ra.items():
            dat_file.write(f"set Ra[{a}]:=\n")
            for r in rooms:
                dat_file.write(f"{r} ")
            dat_file.write(";\n")
        dat_file.write("\n")
        logger.info("DONE")

        logger.info("\tWritting set of student timeslot preferences...")
        for s, tslots in Ts.items():
            dat_file.write(f"set Ts[{s}]:=\n")
            for ts in tslots:
                dat_file.write(f"{ts} ")
            dat_file.write(";\n")
        dat_file.write("\n")
        logger.info("DONE")

        logger.info("\tWritting room capacity...")
        dat_file.write("param rho:=\n")
        
        for r in np.arange(1,nm_rooms+1):
            dat_file.write(f"{r} {rho}\n")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting course capacity...")
        dat_file.write("param sigma_class:=\n")
        for c in np.arange(1,nm_courses+1):
            dat_file.write(f"{int(c)} {sigma_class}\n")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting kmin...")
        dat_file.write("param kmin:=\n")
        for s, k in kmin.items():
            dat_file.write(f"{s} {k}\n")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("\tWritting kmax...")
        dat_file.write("param kmax:=\n")
        for s, k in kmax.items():
            dat_file.write(f"{s} {k}\n")
        dat_file.write(";\n\n")
        logger.info("DONE")

        logger.info("Instancia creada con éxito!")

def parse_probabilistic(output_path):
    import itertools
    n_rooms = 5
    n_blocks = 10
    n_days = 2

    dict_cases = {}

    ### --- 50% capacity
    n_courses = 12
    n_students = 84
    C = [c for c in np.arange(1,n_courses+1)]
    S = [s for s in np.arange(1,n_students+1)]
    T = [f"{day}_{b}_{b+1}" for day, b in itertools.product(range(1,n_days+1), range(1,n_blocks,2))]

    ### --- case 1: each course and timeslot have the same prob. of being chosen
    p_courses = 1/n_courses
    p_tslot = 1/len(T)
    Cs50c1 = {}
    Ts50c1 = {}
    for s in S:
        n_courses_student = np.random.randint(5,8)
        n_ts_student = np.random.randint(2, 5)
        Cs50c1[s] = np.random.choice(C, n_courses_student, replace=False, p=[p_courses]*n_courses)
        Ts50c1[s] = np.random.choice(T, n_ts_student, replace=False, p=[p_tslot]*len(T))

    dict_cases[0] = (C, S, Cs50c1, Ts50c1)

    ### --- case 2: courses follow a multinomial distribution over the total ammount of students
    rng = np.random.default_rng()
    p = rng.multinomial(n_students, [1/12]*12)
    softmax = lambda x: np.exp(x) / np.sum(np.exp(x))
    p_courses = softmax(p)

    Cs50c2 = {}
    Ts50c2 = {}
    for s in S:
        n_courses_student = np.random.randint(5,8)
        n_ts_student = np.random.randint(2, 5)
        Cs50c2[s] = np.random.choice(C, n_courses_student, replace=False, p=p_courses)
        Ts50c2[s] = np.random.choice(T, n_ts_student, replace=False, p=[p_tslot]*len(T))

    dict_cases[1] = (C, S, Cs50c2, Ts50c2)
    
    ### --- 80% capacity
    n_courses = 20
    n_students = 140
    C = [c for c in np.arange(1,n_courses+1)]
    S = [s for s in np.arange(1,n_students+1)]

    ### --- case 1
    p_courses = 1/n_courses

    Cs80c1 = {}
    Ts80c1 = {}
    for s in S:
        n_courses_student = np.random.randint(5,8)
        n_ts_student = np.random.randint(2,5)
        Cs80c1[s] = np.random.choice(C, n_courses_student, replace=False, p=[p_courses]*n_courses)
        Ts80c1[s] = np.random.choice(T, n_ts_student, replace=False, p=[p_tslot]*len(T))

    dict_cases[2] = (C, S, Cs80c1, Ts80c1)

    ### --- case 2: courses follow a multinomial distribution over the total ammount of students
    rng = np.random.default_rng()
    p = rng.multinomial(n_students, [1/n_courses]*n_courses)
    softmax = lambda x: np.exp(x) / np.sum(np.exp(x))
    p_courses = softmax(p)

    Cs80c2 = {}
    Ts80c2 = {}
    for s in S:
        n_courses_student = np.random.randint(5,8)
        n_ts_student = np.random.randint(2, 5)
        Cs80c2[s] = np.random.choice(C, n_courses_student, replace=False, p=p_courses)
        Ts80c2[s] = np.random.choice(T, n_ts_student, replace=False, p=[p_tslot]*len(T))

    dict_cases[3] = (C, S, Cs80c2, Ts80c2)

    ### --- Write 4 instances
    for i in range(4):
        out_file = Path(f"{output_path}_prob{i+1}.dat")
        with open(out_file, "w") as dat_file:
            logger.info(f"\tWritting to {out_file}")
            write_sigmas(dat_file)
            dat_file.write("\n")

            courses, students, Cs, Ts = dict_cases[i]

            rooms = np.arange(1, n_rooms+1)

            activities = np.arange(1, len(courses) * 2+1)
            Ac = {c : [2*c - 1, 2*c] for c in courses}
            Ra = { a : np.arange(1,n_rooms+1) for a in activities}

            rho = sigma_class = len(students)

            kmin = {s : int(len(Cs[s]) * 0.5) for s in students}
            kmax = {s : len(Cs[s]) for s in students}

            logger.info("\tWritting set of students...")
            dat_file.write("set S:=\n")
            for e in students:
                dat_file.write(f"{int(e)} ")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting set of courses...")
            dat_file.write("set C:=\n")
            for c in courses:
                dat_file.write(f"{int(c)} ")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting set of activities...")
            dat_file.write("set A:=\n")
            for act in activities:
                dat_file.write(f"{act} ")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting set of Timeslots...")
            dat_file.write("set T:=\n")
            for i, ts in enumerate(T):
                dat_file.write(f"{ts} ")
                if (i+1) % (5) == 0:
                    dat_file.write("\n")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting set of rooms...")
            dat_file.write("set R:=\n")
            for r in rooms:
                dat_file.write(f"{int(r)} ")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting set of course activities...")
            for c, acts in Ac.items():
                dat_file.write(f"set Ac[{c}]:=\n")
                for a in acts:
                    dat_file.write(f"{a} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            logger.info("DONE")
            
            logger.info("\tWritting set of student course preferenes...")
            for e, s_courses in Cs.items():
                dat_file.write(f"set Cs[{e}]:=\n")
                for c in s_courses:
                    dat_file.write(f"{c} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            logger.info("DONE")

            logger.info("\tWritting set of room availbalities...")
            for a, rooms in Ra.items():
                dat_file.write(f"set Ra[{a}]:=\n")
                for r in rooms:
                    dat_file.write(f"{r} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            logger.info("DONE")

            logger.info("\tWritting set of student timeslot preferences...")
            for s, tslots in Ts.items():
                dat_file.write(f"set Ts[{s}]:=\n")
                for ts in tslots:
                    dat_file.write(f"{ts} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            logger.info("DONE")

            logger.info("\tWritting room capacity...")
            dat_file.write("param rho:=\n")
            
            for r in np.arange(1,n_rooms+1):
                dat_file.write(f"{r} {rho}\n")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting course capacity...")
            dat_file.write("param sigma_class:=\n")
            for c in courses:
                dat_file.write(f"{int(c)} {sigma_class}\n")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting kmin...")
            dat_file.write("param kmin:=\n")
            for s, k in kmin.items():
                dat_file.write(f"{s} {k}\n")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("\tWritting kmax...")
            dat_file.write("param kmax:=\n")
            for s, k in kmax.items():
                dat_file.write(f"{s} {k}\n")
            dat_file.write(";\n\n")
            logger.info("DONE")

            logger.info("Instancia creada con éxito!")

ALGS_LIT = Literal['sori', 'itc', 'prob']
def main(alg: ALGS_LIT, args_dict):

    algs: dict[ALGS_LIT, Callable] = {
        "sori": ("Empezando a crear instancia con algoritmo SORI...", parse_sori),
        "itc": ("Empezando a crear instancia con algoritmo para archivos de la ITC...", parse_itc),
        "prob": ("Creando instancias probabilisticas...", parse_probabilistic)
    }

    msg, func = algs[alg]
    logger.info(msg)
    func(**args_dict)

if __name__ == "__main__":
    
    import argparse
    import threading

    parser = argparse.ArgumentParser(
        prog="General Instance Parser"
    )

    subparser = parser.add_subparsers(dest="algorithm", required=True)

    parser_sori = subparser.add_parser("sori")
    parser_itc = subparser.add_parser("itc")
    parser_prob = subparser.add_parser("prob")

    # SORI algorithm arguments
    parser_sori.add_argument("nm_students", type=int)
    parser_sori.add_argument("nm_courses", type=int)
    parser_sori.add_argument("nm_rooms", type=int)
    parser_sori.add_argument("nm_timeslots", type=int)
    parser_sori.add_argument("rho", type=int)
    parser_sori.add_argument("sigma_class", type=int)
    parser_sori.add_argument("output_path")

    # ITC algorithm arguments
    parser_itc.add_argument("XML_instance")
    parser_itc.add_argument("model_dir")
    parser_itc.add_argument("output")
    parser_itc.add_argument("nm_timeslots", type=int)
    parser_itc.add_argument("room_cap", type=int)
    parser_itc.add_argument("class_lim", type=int)

    # Probabilistic instance arguments
    parser_prob.add_argument("output_path", type=str)

    args = parser.parse_args()

    args_dict = vars(args).copy()
    alg = args_dict.pop("algorithm")

    thread = threading.Thread(name="Instance parser", target=main,
                              args=(alg, args_dict))
    
    thread.start()
    thread.join()
    