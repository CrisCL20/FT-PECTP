import sys
import os
import xml.etree.ElementTree as ET
from typing import Any
import numpy as np

import ITCinstanemodel as itc

def main(instance_file):
    full_path = os.path.join("instances", f"{instance_file}.xml")
    if os.path.exists(full_path):
        tree = ET.parse(full_path)
        root = tree.getroot()

        name, nDays, nSlotsPerDay, nWeeks = root.items()

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
        CLASSES: list[itc.Class] = []
        for c in courses:
            course_id = c.items()[0][1]
            course = itc.Course(course_id)
            for config in c:
                conf_id = config.items()[0][1]
                conf = itc.Config(conf_id)
                for subpart in config:
                    part_id = subpart.items()[0][1]
                    part = itc.Subpart(part_id)
                    ## getting lecture...
                    for lecture in subpart: #it is a class but class is a keyword lol
                        class_attr = lecture.items()
                        lect: itc.Class = None
                        if len(class_attr) == 2: #id, limit
                            class_id, limit = class_attr
                            class_id = class_id[1]
                            limit = limit[1]

                            lect = itc.Class(class_id, limit,room_val=True)

                        elif len(class_attr) == 3: #id, parent, limit or id, limit, room="false"
                            class_attr_ = class_attr
                            class_id = class_attr_[0][1]
                            if class_attr_[1][0] == "parent": #parent, limit
                                parent = class_attr_[1][1]
                                limit = class_attr_[2][1]
                                lect = itc.Class(class_id, int(limit), parent, True)
                            elif class_attr_[1][0] == "limit": #limit, room=false
                                limit = int(class_attr_[1][1])
                                lect = itc.Class(class_id, limit)

                        for attr in lecture:
                            if attr.tag == "room":
                                r_id, penalty = attr.items()
                                r_id = r_id[1]
                                penalty = penalty[1]
                                for room in ROOMS:
                                    if room.id == r_id:
                                        r = lect.add_room(room, int(penalty))
                                        break
                            elif attr.tag == "time":
                                days, start, length, weeks, penalty = attr.items()
                                formatted_days = []
                                formatted_weeks = []
                                for i,d in enumerate(days[1]):
                                    if d == '1':
                                        formatted_days.append(i+1)
                                for i, w in enumerate(weeks[1]):
                                    if w == '1':
                                        formatted_weeks.append(i+1)
                                lect.add_timeset(formatted_days,int(start[1]), 
                                                 int(length[1]), formatted_weeks, 
                                                 int(penalty[1]))
                        CLASSES.append(lect)
                        # adding lecture to subpart
                        part.add_class(lect)
                    # adding part to configuration
                    conf.add_part(part)
                # add config to course
                course.add_config(conf)
            COURSES.append(course)

        distributions = root[3]
        DISTRIBUTIONS: list[itc.Distribution] = []

        for dist in distributions:
            type, req_or_pen = dist.items()
            type = type[1]
            new_dist = itc.Distribution(type)
            if req_or_pen[0] == "required":
                new_dist.set_required(True)
            elif req_or_pen[0] == "penalty":
                penalty = int(req_or_pen[1])
                new_dist.set_penalty(penalty)
            
            for class_ in dist:
                class_id = class_.items()[0][1]
                for class__ in CLASSES:
                    if class__.id == class_id:
                        new_dist.add_class(class__)
            DISTRIBUTIONS.append(new_dist)
        
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
        with open(f"models/instance.dat", "w") as dat_file:
            # Set of students
            print("Calculando conjunto de estudiantes")
            dat_file.write("set S:=\n")
            for student in STUDENTS:
                dat_file.write(student.id+" ")
            dat_file.write(";\n\n")

            # Set of classes
            print("Calculando conjunto de clases")
            dat_file.write("set C:=\n")
            for class_ in CLASSES:
                dat_file.write(class_.id+" ")
            dat_file.write(";\n\n")

            # Set of modules
            print("Calculando conjunto de cursos (ramos)")
            dat_file.write("set M:=\n")
            for module in COURSES:
                dat_file.write(module.id+" ")
            dat_file.write(";\n\n")

            # set of timeslots
            print("Generando conjunto de bloques")
            dat_file.write("set L:=\n")
            # bloques van por dia_idblock donde id block va desde 1-2, 3-4,...,9-10
            # se asume que para todas las semanas es la misma programación
            n_days = 5
            blocks = [f"{i}_{i+1}" for i in range(1,10,2)]
            for day in range(1,n_days+1):
                for id_block in blocks:
                    dat_file.write(f"{day}_{id_block} ")
                dat_file.write("\n")
            dat_file.write(";\n\n")

            # set of rooms
            print("Calculando conjunto de salones")
            dat_file.write("set R:=\n")
            for room in ROOMS:
                dat_file.write(f"{room.id} ")
            dat_file.write(";\n\n")

            # set of modules requested by s in S
            print("Calculando conjunto de cursos que preinscribe s in S")
            for student in STUDENTS:
                dat_file.write(f"set Ms[{student.id}]:=\n")
                for course in student.courses:
                    dat_file.write(f"{course.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            
            # set of unavailable timeslots for each room r in R
            #print("Calculando conjunto de bloques en los que r in R no está disponible")
            #for room in ROOMS:
            #    dat_file.write(f"set RrU[{room.id}]:=\n")
            #    for unavailability in room.unavail_list:
            #        days, start, length, weeks = unavailability
            #        for day in days:
            #            for week in weeks:
            #                for offset in range(length):
            #                    dat_file.write(f"{week}_{day}_{start+offset} ")
            #    dat_file.write(";\n")
            #dat_file.write("\n")

            # set of adequate rooms for class c in C
            print("Conjunto de salones que son adecuados para cada clase")
            for class_ in CLASSES:
                dat_file.write(f"set Rc[{class_.id}]:=\n")
                for room, _ in class_.rooms: # solo un subconjunto de los salones es adecuado
                    dat_file.write(f"{room.id} ")
                if not class_.room or len(class_.rooms) == 0: # todos los salones son adecuados
                    for room in ROOMS:
                        dat_file.write(f"{room.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # time matrix between one room and another
            print("Calculando matriz A")
            dat_file.write("param A:=\n")
            for room in ROOMS:
                for travel in room.travel_list:
                    dest_id, time = travel
                    dat_file.write(f"{room.id} {dest_id} {time} ")
                if len(room.travel_list) == 0:
                    continue
                dat_file.write("\n")
                
            dat_file.write(";\n\n")

            # class per course matrix
            print("Calculando matriz CM")
            dat_file.write("param CM:=\n")
            for course in COURSES:
                for config in course.configs:
                    for part in config.subparts:
                        for class_ in part.classes:
                            dat_file.write(f"{course.id} {class_.id} 1 ")
                dat_file.write("\n")
            dat_file.write(";\n\n")

            # room capacity
            print("Calculando capacidad de salones")
            dat_file.write("param room_cpcty :=\n")
            for room in ROOMS:
                dat_file.write(f"{room.id} {room.capacity}\n")
            dat_file.write(";\n\n")

            print("Calculando limite de clases")
            dat_file.write("param class_limit:=\n")
            for class_ in CLASSES:
                dat_file.write(f"{class_.id} {class_.limit}\n")
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
    #try:
    ifile = sys.argv[1]
    main(ifile)
    print(f"Creada instancia para {ifile}")
    #except Exception as e:
    #    print(f"Error: {e}")