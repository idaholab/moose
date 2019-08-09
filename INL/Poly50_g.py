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
temp=[]
max = thickness.index(max(thickness))
for t in range(0,len(thickness)):
        diff.append(np.array(data[t])-np.array(data[max]))
print(diff)
for t in range(0,len(thickness)):
    if t==max:
        diff.append(np.array(data[t])-np.array(data[max]))
        #norm_x.append(np.linalg.norm(diff[:,0]))
        #norm_y.append(np.linalg.norm(diff[:,1]))
        continue
    diff.append(np.array(data[t])-np.array(data[max]))
    #norm_x.append(np.linalg.norm(diff[:,0]))
    #norm_y.append(np.linalg.norm(diff[:,1]))

temp=(diff[:,1],diff[:,2])
print(temp)
norm_x=sort_list(norm_x,thickness)
norm_y=sort_list(norm_y,thickness)
thickness.sort()
print(thickness)
print(norm_x)
print(norm_y)
plt.plot(thickness,norm_x,norm_y)
plt.show()
#plt.plot(thickness,norm_y)
#plt.show()
