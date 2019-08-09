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
thickness=[]
for file in files:
    thickness.append(float(file.split('_')[1]))
    data.append(pd.read_csv(file, usecols=['u_x','u_y']))

diff=[]
norm_x=[]
norm_y=[]
max = thickness.index(max(thickness))
for t in range(0,len(thickness)):
    if t==max:
        diff=np.array(data[max])-np.array(data[max])
        norm_x.append(np.linalg.norm(diff[:,0]))
        norm_y.append(np.linalg.norm(diff[:,1]))
        continue
    diff=np.array(data[t])-np.array(data[max])
    norm_x.append(np.linalg.norm(diff[:,0]))
    norm_y.append(np.linalg.norm(diff[:,1]))

norm_x=sort_list(norm_x,thickness)
norm_y=sort_list(norm_y,thickness)
thickness.sort()
print(norm_x)
print(norm_y)
plt.plot(thickness,norm_x,norm_y)
plt.show()
#plt.plot(thickness,norm_y)
#plt.show()
