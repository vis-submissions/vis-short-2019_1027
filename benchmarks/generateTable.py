#!/usr/bin/python3

import string

from config import *
import math
import numpy as np
import json
import pandas as pd

import locale


def main():
    print("Generating tables")

    #locale.setlocale(locale.LC_ALL, 'en_US.UTF8')

    '''
    \toprule
    \textbf{Scene Name} & \textbf{Mode} & \multicolumn{2}{c|}{Time Repulsive} & Time Attractive & Time Disperse & \textbf{Total Time} & \textbf{Speed up} \\
    & & Build & Trav. & & & & \\

    \multirow{3}{*}{\shortstack[c]{5K × K5 : 10\\ (connected)}}     & Naive & 0.0 & 0.0 & 0.0 & 0.0 & total & - \\
    \cline{2-8}
               & LBVH & 0.0 & 0.0 & 0.0 & 0.0 & total & - \\
    \cline{2-8}
               & RTX(Ours) & 0.0 & 0.0 & 0.0 & 0.0 & total & \textbf{100x}\\
    \midrule

    \end{tabu}
    '''

    table = ""
    table += "\\begin{tabu}{c|c||c|c|c|c|c|c||c|c} \n"
    table += "\\toprule \n"
    table += "\\textbf{Scene Name} & \\textbf{Mode} & \\multicolumn{2}{c|}{Time Repulsive} & Sum Rep. & BVH Memory & Time Attractive & Time Disperse & \\textbf{Total Time} & \\textbf{Speed up}  \\\\\n"
    table += "& & Build & Trav. & & & & \\\\ \n"
    #table += "\cline{4-17}\n"
    table += "\\hline \n"

    for scene in scenesForTable:
        value = cmdConfigs[scene]

        dRuntimes = {}
        dNodes = {}

        for mode in layoutMode:
            filename = results_folder + "/" + scene + "_" + mode + ".csv"
            df = pd.read_csv(filename, sep=',')
            time = df['Elapsed']
            time_avg = np.average(time[warm_up_iterations:])
            dRuntimes[mode] = time_avg
            numNodes = df['NumNodes'][0]
            if math.isnan(numNodes):
                numNodes = -1

        speedup = dRuntimes["lbvh"] / dRuntimes["rtx"]

        firstIteration = True

        for mode in layoutMode:
            filename = results_folder + "/" + scene + "_" + mode + ".csv"
            df = pd.read_csv(filename, sep=',')

            parseables = ["Iteration", "Elapsed", "TimeRepulsiveForce", "TimeBuildAccel", "TimeTraverseAccel", "NumNodes",
                          "NumEdges", "kValue", "Memory", "Temperature", "TimeAttractiveForces", "TimeDisperse", "TimeCopyBack", "AvgTimeRepulsiveForce"]

            parsedValues = {}
            for p in parseables:
                parsedValues[p] = np.average(df[p])
                if p != "Memory":
                    parsedValues[p] *= 1000

            if firstIteration:
                table += "\\multirow{%d}{*}{%s} " % (len(layoutMode), scene)
                firstIteration = False

            sumtimeRep = parsedValues["TimeBuildAccel"] + \
                parsedValues["TimeTraverseAccel"]
            percentageBuild = parsedValues["TimeBuildAccel"] / sumtimeRep * 100
            percentageTraverse = parsedValues["TimeTraverseAccel"] / \
                sumtimeRep * 100

            table += "& %s " % (mode)
            if mode == 'naive':
                table += "& n/a "
            else:
                table += "& %.2f (%.2f\\%%) " % (
                    parsedValues["TimeBuildAccel"], percentageBuild)

            table += "& %.2f (%.2f\\%%) " % (
                parsedValues["TimeTraverseAccel"], percentageTraverse)

            table += "& %.2f " % (sumtimeRep)

            table += "& %.2f " % (parsedValues["Memory"])
            table += "& %.2f " % (parsedValues["TimeAttractiveForces"])
            table += "& %.2f " % (parsedValues["TimeDisperse"])
            table += "& %.2f " % (parsedValues["Elapsed"])

            if mode == "rtx":
                table += "& \\textbf{%.2f} \\\\ \n" % (speedup)
            else:
                table += "& - \\\\ \n"
                table += "\\cline{2-9} \n"

        table += "\\midrule\n"

    table += "\\end{tabu}\n"

    print(table)


if __name__ == "__main__":
    main()
