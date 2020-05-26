#!/usr/bin/python

import os
import subprocess
import string

from config import *
import numpy as np
import csv
import locale
import json
from benchmarkParser import *


def createDirs(filename):
    if not os.path.exists(os.path.dirname(filename)):
        try:
            os.makedirs(os.path.dirname(filename))
        except OSError as exc: # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise

def run_command(command, params):
    """given shell command, returns communication tuple of stdout and stderr"""
    new_env = os.environ.copy()
    new_env["OWL_PRINT_BVH_MEM"] = "1"
    return subprocess.Popen([command] + params,
                            env=new_env,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            stdin=subprocess.PIPE).communicate()

def main():
    print("Running benchmarks")

    for key, value in cmdConfigs.items():
        for mode in layoutMode:
            print("#######")
            print("##%s -- %s####" % (key, mode))
            print("##############")

            print(mode)

            params = []

            params.append("-bench")
            params.append("-n=%d" % (num_iterations))
            params.append("-mode=%s" % mode)

            commands = value[0]
            files = value[1]

            for option in str.split(commands):
                params.append(option)

            for f in files:
                params.append(f)

            filename=results_folder + "/" + key + "_" + mode
            print(filename)

            print(params)
            # Run command
            rc = run_command(executable, params)
            print(rc[0])
            print(rc[1])

            # Write stdout to file
            stdout_filename = filename + ".out"
            createDirs(stdout_filename)

            f_stdout = open(stdout_filename, "w")
            f_stdout.write(rc[0])
            f_stdout.close()

            # parse stdout
            data = parse_stdout(rc[0]);

            # Wrtie CSV to file

            # Strip away type information
            outputData = {}
            for k, v in data.items():
                arr = v[1]
                if len(arr) < num_iterations:
                    arr = [None] * (num_iterations - len(arr))
                outputData[k] = arr

            zd = zip(*outputData.values())
            with open(filename + ".csv", 'w') as file:
                writer = csv.writer(file, delimiter=',')
                writer.writerow(outputData.keys())
                writer.writerows(zd)

            # Write JSON to file
            data["scene"] = key
            data["options"] = value
            data["stdout"] = stdout_filename
            data["mode"] = mode

            with open(filename+".json", 'w') as outfile:
                json.dump(data, outfile)

if __name__ == "__main__":
    main()
