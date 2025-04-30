class Room:
    def __init__(self, id, capacity: int):
        self.id = id
        self.capacity = capacity
        self.travel_list = []
        self.unavail_list = []

    def add_travel_time(self, dest_id, n_slots: int):
        self.travel_list.append((dest_id, n_slots))
    
    def add_unavailability(self, days: list[int], start: int, length: int, weeks: list[int]):
        self.unavail_list.append((days,start,length, weeks))
    
    def print(self,prefix=""):
        print(prefix+f"Room: {self.id}, cap: {self.capacity}")
        print(prefix+"Travel list:")
        if len(self.travel_list) > 0:
            for travel in self.travel_list:
                dest_id, n_slots = travel
                print(prefix+f"Dest: {dest_id}, time: {n_slots}")
        print(prefix+"Unavailability:")
        if len(self.unavail_list) > 0:
            for unavail in self.unavail_list:
                days, start, length, weeks = unavail
                print(prefix+f"Days: {days}")
                print(prefix+f"Start: {start}, length: {length}")
                print(prefix+f"Weeks: {weeks}")  

class Class:
    def __init__(self, id, limit, parent_id=None, room_val = False):
        self.id = id
        self.limit = limit
        self.rooms: list[tuple[Room, int]] = []
        self.room = room_val #Este atributo no está presente en el xml cuando len(rooms) > 0
        self.parent = parent_id
        self.timesets: list = []

    def add_room(self, room: Room, penalty):
        if not self.room:
            self.room = True
        self.rooms.append((room, penalty))
    
    def add_timeset(self, days, start, length, weeks, penalty):
        self.timesets.append((days, start, length, weeks, penalty))
    
    def print(self,prefix):
        print(f"Class: {self.id}, limit: {self.limit}")
        print("Rooms:")
        for room, _ in self.rooms:
            room.print(prefix+"\t")
        print(f"parent: {self.parent}")
        print(f"timesets: {self.timesets}")

class Subpart:
    def __init__(self, id):
        self.id = id
        self.classes: list[Class] = []
    
    def add_class(self, lect: Class):
        self.classes.append(lect)

class Config:
    def __init__(self, id):
        self.id = id
        self.subparts: list[Subpart] = []
    def add_part(self, part: Subpart):
        self.subparts.append(part)

class Course:
    def __init__(self, id):
        self.id = id
        self.configs: list[Config] = []
    def add_config(self, conf: Config):
        self.configs.append(conf)
    
    def print(self):
        print(f"id: {self.id}")
        print("Configurations:")
        for conf in self.configs:
            print(f"config_id: {conf.id}")
            print("Subparts:")
            for part in conf.subparts:
                print(f"\tpart: {part.id}")
                print("\tClasses:")
                for lect in part.classes:
                    lect.print("\t\t")

class Student:
    def __init__(self,id):
        self.id = id
        self.courses: list[Course] = []
    
    def add_course(self, course: Course):
        self.courses.append(course)

class Distribution:
    def __init__(self, type):
        self.type = type
        self.penalty = None
        self.required = None
        self.classes: list[Class] = []
    
    def set_penalty(self, penalty):
        self.set_required(False)
        self.penalty = penalty
    
    def set_required(self, required):
        self.required = required
    
    def add_class(self, lect: Class):
        self.classes.append(lect)
