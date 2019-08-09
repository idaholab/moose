#!/usr/bin/env python
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import sys
from mpl_toolkits.mplot3d import axes3d
import matplotlib
from matplotlib.cm import coolwarm


def sort_list(list1, list2):
    zipped_pairs = zip(list2, list1)
    z = [x for _, x in sorted(zipped_pairs)]
    return z
files = sys.argv[1:]
data = []
mat=[]
thickness=[]
coordinate_x=[]
for file in files:
    thickness.append(float(file.split('_')[1]))
    data.append(pd.read_csv(file, usecols=['x','y','u_x','u_y']))
    #coordinates.append(np.array(data[1]))
diff=[]
mat=np.array(data[1])
coordinate_x=mat[:,0]
coordinate_y=mat[:,1]

print(coordinate_x)
norm_x=[]
norm_y=[]
temp_x=[]
temp_y=[]
max = thickness.index(max(thickness))
for t in range(0,len(thickness)):
        diff.append(np.array(data[t])-np.array(data[max]))
        if t==max:
            continue
        temp_x.append(diff[t][:,2])
        temp_y.append(diff[t][:,3])
print(len(coordinate_x))
print(temp_x)
print(temp_y)
sort_list(coordinate_y,coordinate_x)

for t in range(0,len(thickness)-1):
    sort_list(temp_x[t],coordinate_x)
    #plt.hist2d(coordinate_x, coordinate_y, bins=N_bins, normed=False, cmap='plasma')
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    #ax=fig.gca(projection='3d')
    #ax = Axes3D(fig)
    coordinate_x.sort()
    #X,Y = np.meshgrid(coordinate_x,coordinate_y)
    #ax = fig.add_subplot(111, projection='3d')
    #ax.scatter(coordinate_x, coordinate_y, temp_x[t])
    #ax.set_xlabel('X ')
    #ax.set_ylabel('Y ')
    #ax.set_zlabel('Z ')
    #plt.show()
    #X, Y = np.meshgrid(coordinate_x, coordinate_y)
    #surf = ax.plot_surface(X, Y, temp_x[t], cmap=coolwarm, linewidth=0,antialiased=False)
    #x,y,z=np.meshgrid(coordinate_x,coordinate_y)
    ax.scatter(coordinate_x,coordinate_y,temp_x[t])
    #fig.colorbar(surf, shrink=0.5, aspect=5)
    #plt.figure()
    #plt.contourf(X,Y,temp_x[t])
    #plt.show()
    plt.show()
#plt.plot(thickness,norm_y)
#plt.show()
