import re
import sys

def main(args):
    runnerfile = args["runnerfile"]
    instancefile = args["instance"]

    with open(runnerfile, "r") as file:
        for line in file:
            if re.match(r'^data\s+', line.strip()):
                newline = f"data {instancefile}"
                sys.stdout.write(newline+"\n")
                continue
            
            sys.stdout.write(line)
            
    return 1

if __name__ == "__main__":
    import argparse
    import threading

    parser = argparse.ArgumentParser(
        prog="Set instance file to runner"
    )

    runnerfile = parser.add_argument(
        "runnerfile",
        type=str
    )

    instance = parser.add_argument(
        "instance",
        type=str
    )

    args = parser.parse_args()

    thread = threading.Thread(name="Instance parser", target=main,
                              args=(vars(args),))
    
    thread.start()
    thread.join()
    