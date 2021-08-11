#!/opt/moose/miniconda/bin/python
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib import rc
import matplotlib.patches as mpatches
import math
import glob, os
from numpy import cumsum
#Al's stuff
# import matplotlib as mpl
import numpy as np
from scipy import stats

SMALL_SIZE = 8
MEDIUM_SIZE = 10
BIGGER_SIZE = 12

plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=SMALL_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=7)    # legend fontsize
plt.rc('figure', titlesize=SMALL_SIZE)  # fontsize of the figure title

line_colors=['red', 'blue','green','cyan']
marker_styles=['o', 's','*','.']

fig = plt.figure(figsize=(7.25, 2.5))

ax = fig.add_subplot(1,2,1)
for i in range(1,5):
    filename='iter'+ str(i) +'_forward0_vertical_1_0001.csv'
    df=pd.read_csv(filename)
    df_time=df.loc[:,'temperature']
    df_data=df.loc[:,'y']
    ax.plot(df_time.iloc[:], df_data.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i-1], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label='iter '+str(i))
ax.plot([207, 185], [0.2, 1.4],\
        linestyle='None',marker='o',color='black', alpha=1.0, \
        fillstyle = 'none')

ax.title.set_text('Line Sample x=0.2')
ax.set_xlabel('Temperature')
ax.set_ylabel('y location')

ax = fig.add_subplot(1,2,2)
for i in range(1,5):
    filename='iter'+ str(i) +'_forward0_vertical_2_0001.csv'
    df=pd.read_csv(filename)
    df_time=df.loc[:,'temperature']
    df_data=df.loc[:,'y']
    ax.plot(df_time.iloc[:], df_data.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i-1], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label='iter '+str(i))
ax.plot([204, 125], [0.6, 1.8],\
        linestyle='None',marker='o',color='black', alpha=1.0, \
        fillstyle = 'none')

ax.title.set_text('Line Sample x=0.8')
ax.set_xlabel('Temperature')
ax.set_yticklabels([])
ax.legend()

fig.savefig('lineSamplerConvergence.pdf', format='pdf', bbox_inches='tight', pad_inches=0.1)

#----------------------------------
fig = plt.figure(figsize=(7.25, 2.5))

prefix=['nm', 'cg', 'lmvm']

ax = fig.add_subplot(1,2,1)
for i in range(3):
    filename=prefix[i]+'iter_optInfo_0001.csv'
    df=pd.read_csv(filename)
    df_x=df.loc[:,'current_iterate']
    df_y=df.loc[:,'function_value']
    ax.plot(df_x.iloc[:], df_y.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label=prefix[i])
ax.set_yscale('log')
ax.set_xlabel('iteration')
ax.set_ylabel('objective value')

ax = fig.add_subplot(1,2,2)
for i in range(3):
    filename=prefix[i]+'iter_optInfo_0001.csv'
    df=pd.read_csv(filename)
    df_x=df.loc[:,'gradient_iterate']+df.loc[:,'hessian_iterate']+df.loc[:,'objective_iterate']
    df_y=df.loc[:,'function_value']
    ax.plot(df_x.iloc[:].cumsum(), df_y.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label=prefix[i])
ax.set_yscale('log')
ax.set_xlabel('total solves')
ax.set_yticklabels([])
ax.legend()

fig.savefig('lineSamplerConvergenceComparison.pdf', format='pdf', bbox_inches='tight', pad_inches=0.1)

plt.show()
