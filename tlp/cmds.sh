# tree16
./gd -dt=tree -trDepth=16

# k5:10 unconnected
./gd -dt=artificial -C=50000 -connected=false -epc=10 -npc=5

# k5:10 connected
./gd -dt=artificial -C=5000 -connected=true -epc=10 -npc=5
