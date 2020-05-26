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

def main():
    print("Parsing benchmarks")

    for key, value in cmdConfigs.items():
        for mode in layoutMode:
            print("#######")
            print("##%s -- %s####" % (key, mode))
            print("##############")

            filename=results_folder + "/" + key + "_" + mode
            print(filename)

            # Read stdout from file
            stdout_filename = filename + ".out"


    	    # parse stdout
            with open (stdout_filename, "r") as infile:
                stdoutStr=infile.read()

            data = parse_stdout(stdoutStr);

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
