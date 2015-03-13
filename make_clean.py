import os
import shutil

folder1 = '/home/talbpaul/projects_falcon/crow/build/'
folder2 = '/home/talbpaul/projects_falcon/crow/install/'
folder3 = '/home/talbpaul/projects_falcon/crow/crow_modules/'
folder4 = '/home/talbpaul/projects_falcon/crow/control_modules/'
folder5 = '/home/talbpaul/projects_falcon/crow/control_modules/.libs/'

shutil.rmtree(folder1)
shutil.rmtree(folder2)
shutil.rmtree(folder5)

filelist1 = [ f for f in os.listdir(folder3) if f.endswith(".cpp") ]
for files in filelist1:
  os.remove(folder3 + files)

os.remove(folder3 + "interpolationNDpy2.py")
os.remove(folder3 + "distribution1Dpy2.py")

filelist3 = [ f for f in os.listdir(folder4) if f.endswith(".so") ]
for files in filelist3:
    os.remove(folder4+files)

filelist31 = [ f for f in os.listdir(folder4) if f.endswith(".cxx") ]
for files in filelist31:
    os.remove(folder4+files)

filelist4 = [ f for f in os.listdir(folder4) if f.endswith(".lo") ]
for files in filelist4:
    os.remove(folder4+files)

filelist5 = [ f for f in os.listdir(folder4) if f.endswith(".la") ]
for files in filelist5:
    os.remove(folder4+files)

filelist6 = [ f for f in os.listdir(folder4) if f.endswith(".py") ]
for files in filelist6:
    os.remove(folder4+files)

filelist7 = [ f for f in os.listdir(folder4) if f.endswith(".dylib") ]
for files in filelist7:
    os.remove(folder4+files)
