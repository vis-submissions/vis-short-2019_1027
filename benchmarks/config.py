
from collections import *

results_folder = "/local/owlspringembedder/build/results"
#results_folder = "../results"
num_iterations = 10000
warm_up_iterations = 0

executable = "../build/gd"

layoutMode = ["naive", "lbvh", "rtx"]
#layoutMode = ["lbvh", "rtx"]


##
#Generate runs for benchmarks
cmdConfigs = OrderedDict()
cmdConfigs["gephi-100k"] = ["-r=1", ["../data/gephi/gephi_100k.csv"]]
cmdConfigs["tree16"] = ["-r=1 -dt=tree -trDepth=16", []]
cmdConfigs["k-5-10-unconnected"] = ["-r=1 -dt=artificial -C=50000 -connected=false -epc=10 -npc=5", []]
cmdConfigs["k-5-10-connected"] = ["-r=1 -dt=artificial -C=5000 -connected=true -epc=10 -npc=5", []]

#Add more runs to determine behavior when increasing the number of nodes tree4 - treen
for i in range(4,19,1):
    cmdConfigs["tree%d" % (i)] = ["-r=1 -dt=tree -trDepth=%d" %(i), []]

##
#Determines the scenes for the table generation
scenesForTable = ["gephi-100k", "tree16", "k-5-10-unconnected", "k-5-10-connected"]

#cmdConfigs["gephi-196k"] = ["-r=1", ["../data/gephi/gephi_196k.csv"]]
#cmdConfigs["gephi-100k"] = ["-r=1", ["../data/gephi/gephi_100k.csv"]]
#cmdConfigs["Deezer-R0"] = ["-r=1", ["../data/deezer_clean_data/RO_edges.csv"]]
#cmdConfigs["Deezer-HR"] = ["-r=1", ["../data/deezer_clean_data/HR_edges.csv"]]
#cmdConfigs["Deezer-HU"] = ["-r=1", ["../data/deezer_clean_data/HU_edges.csv"]]
#cmdConfigs["Artificial1"] = ["", ""]
#cmdConfigs["Artificial2"] = ["", ""]
