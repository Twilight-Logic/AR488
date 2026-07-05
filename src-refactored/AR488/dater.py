#!/usr/bin/python3
import os
import sys



def replaceDetails(line, version, vdate) :
    print("Old: ", line, end="")
    expr = "ver."
    epos = line.find(expr)
    vpos = epos + 5
    dpos = epos + 14
    line = line[:vpos] + version + line[(vpos + len(version)):] 
    line = line[:dpos] + vdate + line[(dpos + len(vdate)):]     
    print("New: ", line, end="")
    return line


def processFile(fname, version, vdate) :
    fdata = ''
    expr1 = "AR488 GPIB Interface"
    expr2 = "define FW_VERSION"
    upd = False
    with open(fname) as f:
        for line in f:
            if ( (line.find(expr1) > 0) or (line.find(expr2) > 0) ) :
                line = replaceDetails(line, version, vdate)
                if (line) : fdata += line 
                upd = True
            else:
                fdata += line
    if (upd):
        with open(fname, 'w') as f:
            f.write(fdata)
        print("Updated: ", fname)
    else:
        print("Not updated.")       




# Get version and date from command line
if len(sys.argv) < 3 :
    print("Please specify version and date!")
    exit(0)

if sys.argv[1] : version = sys.argv[1]
if sys.argv[2] : vdate = sys.argv[2]
print("New version details: ",version, vdate)

# Get the current path
currentPath = os.path.abspath(os.getcwd())
print("Current path: '", currentPath, "'")

# Get the current list of files
dirList = os.listdir(currentPath)

# Extract .ino, .h and .cpp only
for f in dirList :
    if f.endswith(".ino") or f.endswith(".h") or f.endswith(".cpp") :
        print("Processing: ", f)
        processFile(f, version, vdate)
        
        
        
