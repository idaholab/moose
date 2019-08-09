#!/usr/bin/env python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys

def sort_list(list1, list2):
    zipped_pairs = zip(list2, list1)
    z = [x for _, x in sorted(zipped_pairs)]
    return z

files = sys.argv[1:]
data = []
mat=[]
thickness=[]
coordinates=[]
for file in files:
    thickness.append(float(file.split('_')[1]))
    data.append(pd.read_csv(file, usecols=['x','u_x','u_y']))
    #coordinates.append(np.array(data[1]))
diff=[]
mat=np.array(data[1])
coordinates=mat[:,0]
print(coordinates)
norm_x=[]
norm_y=[]
temp_x=[]
temp_y=[]
max = thickness.index(max(thickness))
for t in range(0,len(thickness)):
        diff.append(np.array(data[t])-np.array(data[max]))
        if t==max:
            continue
        temp_x.append(abs((diff[t][:,1])))
        temp_y.append(abs((diff[t][:,2])))
print(len(coordinates))
print(temp_x)
print(temp_y)

for t in range(0,len(thickness)-1):
    sort_list(coordinates,temp_x[t])
    coordinates.sort()
    plt.plot(coordinates,temp_x[t])
    plt.show()
#plt.plot(thickness,norm_y)
#plt.show()
