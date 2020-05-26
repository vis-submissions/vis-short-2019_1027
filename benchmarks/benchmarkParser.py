import string
import os
import subprocess
import string

from config import *
import numpy as np

def parse_float(s):
    try:
        return float(s)
    except ValueError:
        return -1.0

def parse_int(s):
    try:
        return int(s)
    except ValueError:
        return 0

def parse_stdout(stdout_str):
    #This dict contains a list of tokens to parse for and the expected type to parse
    data = {"Iteration": ("int", []),
            "Elapsed" : ("float", []),
            "TimeRepulsiveForce" : ("float", []),
            "TimeBuildAccel" : ("float", []),
            "TimeTraverseAccel" : ("float", []),
            "NumNodes" : ("int", []),
            "NumEdges" : ("int", []),
            "kValue" : ("float", []),
            "Temperature" : ("float", []),
            "TimeAttractiveForces" : ("float", []),
            "TimeDisperse" : ("float", []),
            "TimeCopyBack" : ("float", []),
            "AvgTimeRepulsiveForce" : ("float", []),
            "Memory" : ("float", []),
         #   "Discrepancy" : ("float", [])
            }


    for line in stdout_str.split('\n'):
        line.strip()	#Remove whitespaces
        tokens = line.split(':')

        #Sort it into the dictonary arrays pased on the specified data type
        if tokens[0] in data:
            if data[tokens[0]][0] == "int":
                data[tokens[0]][1].append(parse_int(tokens[1]))
            if data[tokens[0]][0] == "float":
                data[tokens[0]][1].append(parse_float(tokens[1]))
            if data[tokens[0]][0] == "string":
                data[tokens[0]][1].append(tokens[1])
        elif "owl" in tokens[0]:
            tokens = tokens[2].split(',')
            byteStr = tokens[3]
            tokens = byteStr.split("Mb")
            if len(tokens) < 2:
                tokens = byteStr.split("Kb")
                # Convert to MB
                data["Memory"][1].append(parse_float(tokens[0])/1024.0)
            else:
                data["Memory"][1].append(parse_float(tokens[0]))

    time_avg = 0.0
    if len(data["Elapsed"]) > 0:
	    time_avg = np.average(data["Elapsed"][1][warm_up_iterations:])
    print("Avg. Elapsed Time: %lf\n" % (time_avg))

    return data
