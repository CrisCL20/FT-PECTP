import sys
import os
import xml.etree.ElementTree as ET
from typing import Any

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
            print("Calculando conjunto de modulos")
            dat_file.write("set M:=\n")
            for module in COURSES:
                dat_file.write(module.id+" ")
            dat_file.write(";\n\n")

            # set of timeslots
            print("Calculando conjunto de bloques")
            dat_file.write("set L:=\n")
            for week in range(1,int(nWeeks[1])+1):
                for day in range(1,int(nDays[1])+1):
                    for tslot in range(1,int(nSlotsPerDay[1])+1):
                        block_id = f"{week}_{day}_{tslot}"
                        dat_file.write(block_id+" ")
            dat_file.write(";\n\n")
            
            # set of timesets
            print("Calculando conjunto de horarios")
            timesets: dict[Any,list] = {}
            for class_ in CLASSES:
                for timeset in class_.timesets:
                    days,start,length, weeks,_ = timeset
                    for day in days:
                        for week in weeks:
                            for slot_offset in range(length):
                                ts = f"{week}_{day}_{start+slot_offset}"
                                ### each unique timeset in the classes will be out timesets
                                if class_.id not in timesets:
                                    timesets[class_.id] = []
                                if ts not in timesets[class_.id]:
                                    timesets[class_.id].append(ts)

            dat_file.write("set T:=\n")
            for i, timeset in timesets.items():
                dat_file.write(f"t{i} ")
            dat_file.write(";\n\n")

            for i, tsets in timesets.items():
                dat_file.write(f"set L_in_T[t{i}]:=\n")
                for tset in tsets:
                    dat_file.write(f"{tset} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of rooms
            print("Calculando conjunto de salones")
            dat_file.write("set R:=\n")
            for room in ROOMS:
                dat_file.write(f"{room.id} ")
            dat_file.write(";\n\n")

            # set of modules requested by s in S
            print("Calculando conjunto de módulos a los que va cada s in S")
            for student in STUDENTS:
                dat_file.write(f"set Ms[{student.id}]:=\n")
                for module in student.courses:
                    dat_file.write(f"{module.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")
            
            # set of obligatory modules for s in S
            # (we will make none of them mandatory, so Mobs=Mels)
            print("Calculando conjunto de modulos electivos.")
            for student in STUDENTS:
                dat_file.write(f"set Mels[{student.id}]:=\n")
                for module in student.courses:
                    dat_file.write(f"{module.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of students that want to attend module m in M
            print("Calculando conjunto de estudiantes que quieren ir a un módulo m")
            students_per_module = {}
            for student in STUDENTS:
                for module in student.courses:
                    if module.id not in students_per_module:
                        students_per_module[module.id] = []
                    if student.id not in students_per_module[module.id]:
                        students_per_module[module.id].append(student.id)
            
            for module_id, students in students_per_module.items():
                dat_file.write(f"set Sm[{module_id}]:=\n")
                for student_id in students:
                    dat_file.write(f"{student_id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of unavailable timeslots for each room r in R
            print("Calculando conjunto de bloques en los que r in R no está disponible")
            for room in ROOMS:
                dat_file.write(f"set RrU[{room.id}]:=\n")
                for unavailability in room.unavail_list:
                    days, start, length, weeks = unavailability
                    for day in days:
                        for week in weeks:
                            for offset in range(length):
                                dat_file.write(f"{week}_{day}_{start+offset} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of adequate rooms for course c in C
            print("Conjunto de salones que son adecuados para cada curso")
            for class_ in CLASSES:
                dat_file.write(f"set Rc[{class_.id}]:=\n")
                for room, _ in class_.rooms: # solo un subconjunto de los salones es adecuado
                    dat_file.write(f"{room.id} ")
                if not class_.room: # todos los salones son adecuados
                    for room in ROOMS:
                        dat_file.write(f"{room.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # Set of adequate timesets for class c in C
            print("Conjunto de horarios adecuados para cada clase c in C")
            for class_ in CLASSES:
                dat_file.write(f"set Tc[{class_.id}]:=t{class_.id};\n") 
            dat_file.write("\n")

            # set of configurations for each module m in M
            print("Calculando configuraciones para cada módulo")
            for module in COURSES:
                dat_file.write(f"set Fm[{module.id}]:=\n")
                for config in module.configs:
                    dat_file.write(f"{config.id} ")
                dat_file.write(";\n")
            dat_file.write("\n")

            # set of subparts in each config
            print("Calculando conjunto de subpartes para cada configuración")
            for module in COURSES:
                for config in module.configs:
                    dat_file.write(f"set P[{module.id}, {config.id}]:=")
                    for part in config.subparts:
                        dat_file.write(f"{part.id} ")
                    dat_file.write(";\n")
            dat_file.write("\n")

            # set of classes in part p of config f of module m
            print("Calculando Cmfp")
            for module in COURSES:
                for config in module.configs:
                    for part in config.subparts:
                        dat_file.write(f"set Cmfp[{module.id}, {config.id}, {part.id}]:=\n")
                        for class_ in part.classes:
                            dat_file.write(f"{class_.id} ")
                        dat_file.write(";\n")
            dat_file.write("\n")

            print("Calculando matriz A")
            dat_file.write("param A:=\n")
            for room in ROOMS:
                for travel in room.travel_list:
                    dest_id, time = travel
                    dat_file.write(f"{room.id} {dest_id} {time} ")
                dat_file.write("\n")
            dat_file.write(";\n\n")

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

            #D0[r,t] = 1 si r in R no está disponible en el horario t in T
            print("Calculando D0")
            dat_file.write("param D0:=\n")

            # Preprocesar todas las indisponibilidades por sala
            room_unavailabilities = {room.id: set() for room in ROOMS}
            for room in ROOMS:
                for unavail in room.unavail_list:
                    days, start, length, weeks = unavail
                    for day in days:
                        for week in weeks:
                            for offset in range(length):
                                block = f"{week}_{day}_{start + offset}"
                                room_unavailabilities[room.id].add(block)

            # Verificar para cada sala y timeset
            for room in ROOMS:
                for class_id, timeset_blocks in timesets.items():
                    timeset_id = f"t{class_id}"
                    # Si algún bloque del timeset está en las indisponibilidades de la sala
                    if any(block in room_unavailabilities[room.id] for block in timeset_blocks):
                        dat_file.write(f"[{room.id},{timeset_id}] 1\n")

            dat_file.write(";\n\n")

            #D1[t1,t2] = 1 si t1 choca con t2
            # print("Calculando D1")
            # dat_file.write("param D1:=\n")

            # # Convertir timesets a conjuntos para operaciones eficientes
            # timeset_blocks = {f"t{class_id}" : set(blocks) for class_id, blocks in timesets.items()}

            # # Comparar todos los pares únicos de timesets
            # timeset_ids = list(timeset_blocks.keys())
            # for i in range(len(timeset_ids)):
            #     t1 = timeset_ids[i]
            #     for j in range(i + 1, len(timeset_ids)):  # Evitar comparar con sí mismo y duplicados
            #         t2 = timeset_ids[j]
            #         if len(timeset_blocks[t1] & timeset_blocks[t2]) == 0:  # Intersección no vacía
            #             dat_file.write(f"[{t1},{t2}] 0\n")

            # dat_file.write(";\n\n")

            #D2[r1,r2,t1,t2] = 1 si no hay tiempo entre t1 y t2 para
            # ir de r1 a r2
            
            # print("Calculando D2")
    
            # D2 = {}
            # # Primero creamos un mapeo de clase a sus timesets y bloques
            # class_timesets = {}
            # for class_ in CLASSES:
            #     timeset_id = f"t{class_.id}"
            #     blocks = []
            #     for timeset in class_.timesets:
            #         days, start, length, weeks, _ = timeset
            #         block_l = f"{weeks[0]}_{days[0]}_{start}"
            #         block_u = f"{weeks[-1]}_{days[-1]}_{start+length-1}"
            #         blocks.append((block_l, block_u))
            #     class_timesets[class_.id] = (timeset_id, blocks)
            
            # travel_times = {}
            # for room in ROOMS:
            #     for dest_id, n_slots in room.travel_list:
            #         travel_times[(room.id, dest_id)] = n_slots

            # for i, c1 in enumerate(CLASSES):
            #     for j, c2 in enumerate(CLASSES):
            #         if i >= j:  # Evitamos duplicados ya que D2 es simétrica
            #             continue
                        
            #         # Obtenemos timesets y bloques de cada clase
            #         t1_id, blocks1 = class_timesets[c1.id]
            #         t2_id, blocks2 = class_timesets[c2.id]
                    
            #         # Encontramos el último bloque de c1 y el primero de c2
            #         last_block_c1 = blocks1[-1][1]
            #         first_block_c2 = blocks2[0][0]
                    
                    
            #         # Parseamos los bloques para obtener semana, día y slot
            #         _,_, slot1 = map(int, last_block_c1.split('_'))
            #         _,_, slot2 = map(int, first_block_c2.split('_'))
            #         time_between = abs(slot1 - slot2)
            #         print(time_between)
            #         # Verificamos para todos los pares de salones posibles
            #         for r1, _ in c1.rooms:
            #             for r2, _ in c2.rooms:
            #                 # Obtenemos tiempo de viaje requerido
                            
            #                 required_time = travel_times.get((r1.id, r2.id),0)
                            
                         
            #                 if time_between > required_time:
            #                     if r1.id not in D2:
            #                         D2[r1.id] = {}
            #                     if r2.id not in D2[r1.id]:
            #                         D2[r1.id][r2.id] = {}
            #                     if t1_id not in D2[r1.id][r2.id]:
            #                         D2[r1.id][r2.id][t1_id] = {}
                                
            #                     D2[r1.id][r2.id][t1_id][t2_id] = 0

    
            # dat_file.write("param D2 :=\n")
            # for r1 in D2:
            #     for r2 in D2[r1]:
            #         for t1 in D2[r1][r2]:
            #             for t2 in D2[r1][r2][t1]:
            #                 if D2[r1][r2][t1][t2] == 0:
            #                     dat_file.write(f"[{r1},{r2},{t1},{t2}] 0\n")
            
            # dat_file.write(";\n\n")
            
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