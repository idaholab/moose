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
iterations=['1','7','14','21']


fig = plt.figure(figsize=(7.25, 2.5))

ax = fig.add_subplot(1,2,1)
for i in range(4):
    filename='lmvmiter'+iterations[i]+'_forward0_horizontal_0001.csv'
    print(filename)
    df=pd.read_csv(filename)
    df_y=df.loc[:,'temperature']
    df_x=df.loc[:,'x']
    ax.plot(df_x.iloc[:], df_y.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i-1], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label='iter '+iterations[i])
ax.plot([0.2, 0.5, 1.5, 1.8], [210, 220, 160, 120],\
        linestyle='None',marker='o',color='black', alpha=1.0, \
        fillstyle = 'none')
ax.title.set_text('Line Sample y=0.5')
ax.set_ylabel('Temperature')
ax.set_xlabel('x coordinate')
ax.set_ylim([110, 290])

ax = fig.add_subplot(1,2,2)
for i in range(4):
    filename='lmvmiter'+iterations[i]+'_forward0_horizontal2_0001.csv'
    print(filename)
    df=pd.read_csv(filename)
    df_y=df.loc[:,'temperature']
    df_x=df.loc[:,'x']
    ax.plot(df_x.iloc[:], df_y.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i-1], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label='iter '+iterations[i])
ax.plot([0.3, 0.6, 0.9, 1.2], [260, 271, 265, 236],\
        linestyle='None',marker='o',color='black', alpha=1.0, \
        fillstyle = 'none')

ax.title.set_text('Line Sample y=1.1')
ax.set_xlabel('x coordinate')
ax.set_ylim([110, 290])
ax.set_yticklabels([])
ax.legend()

fig.savefig('BodyLoadlineSamplerConvergence.pdf', format='pdf', bbox_inches='tight', pad_inches=0.1)

#----------------------------------
fig = plt.figure(figsize=(7.25, 3))
line_colors=['red', 'magenta','orchid','green','darkgreen','lime','blue','lightblue','navy']
fileprefix=['lmvm','lmvm2','lmvm3','cg','cg2','cg3','nm','nm2','nm3']
prefix=['lmvm','lmvm','lmvm','cg','cg','cg','nm','nm','nm']
suffix=[' pts 1',' pts 2', ' pts 1&2',' pts 1',' pts 2', ' pts 1&2',' pts 1',' pts 2', ' pts 1&2']
# ax = fig.add_subplot(1,1,1)
# for i in range(3):
#     filename=prefix[i]+'3iter_optInfo_0001.csv'
#     df=pd.read_csv(filename)
#     df_x=df.loc[:,'current_iterate']
#     df_y=df.loc[:,'function_value']
#     ax.plot(df_x.iloc[:], df_y.iloc[:],\
#             linestyle='-',marker='.',color=line_colors[i], alpha=1.0, \
#             fillstyle = 'none',linewidth=1, label=prefix[i])
# ax.set_yscale('log')
# ax.set_xlabel('iteration')
# ax.set_ylabel('objective value')

ax = fig.add_subplot(1,1,1)
for i in range(9):
    filename=fileprefix[i]+'iter_optInfo_0001.csv'
    df=pd.read_csv(filename)
    df_x=df.loc[:,'gradient_iterate']+df.loc[:,'hessian_iterate']+df.loc[:,'objective_iterate']
    df_y=df.loc[:,'function_value']
    ax.plot(df_x.iloc[:].cumsum(), df_y.iloc[:],\
            linestyle='-',marker='.',color=line_colors[i], alpha=1.0, \
            fillstyle = 'none',linewidth=1, label=prefix[i]+suffix[i])
ax.set_yscale('log')
ax.set_xlabel('total solves')
ax.set_ylabel('objective value')
ax.legend()

fig.savefig('BodyLoadlineSamplerConvergenceComparison.pdf', format='pdf', bbox_inches='tight', pad_inches=0.1)


plt.show()

#
# fig = plt.figure(figsize=(7.25, 3))
#
# ax = fig.add_subplot(1,3,1)
# ax.title.set_text('Time=10')
# name='*_csv_out_vert_slice_0005.csv'
# all_files=glob.glob(name)
# for fn in all_files:
#     print('filename: ',fn)
#     df=pd.read_csv(fn)
#     df_time=df.loc[:,'y']
#     df_data=df.loc[:,'matrix_T']
#     ax.plot(df_time.iloc[:],\
#             df_data.iloc[:],\
#             '*-', alpha=1.0, linewidth=1)
#
# ax = fig.add_subplot(1,3,2)
# ax.title.set_text('Time=50')
# name='*_csv_out_vert_slice_0025.csv'
# all_files=glob.glob(name)
# for fn in all_files:
#     df=pd.read_csv(fn)
#     df_time=df.loc[:,'y']
#     df_data=df.loc[:,'matrix_T']
#     ax.plot(df_time.iloc[:],\
#             df_data.iloc[:],\
#             '*-', alpha=1.0, linewidth=1)
#
# ax = fig.add_subplot(1,3,3)
# ax.title.set_text('Time=100')
# name='*_csv_out_vert_slice_0050.csv'
# all_files=glob.glob(name)
# for fn in all_files:
#     df=pd.read_csv(fn)
#     df_time=df.loc[:,'y']
#     df_data=df.loc[:,'matrix_T']
#     ax.plot(df_time.iloc[:],\
#             df_data.iloc[:],\
#             '*-', alpha=1.0, linewidth=1)



#
#
# for i in range(3):
#     ax = fig2.add_subplot(1,3,i+1)
#     ax2a.title.set_text('ROM: '+str(pressList[i])+'MPa')
#     ax2b = fig2.add_subplot(2,3,i+4)
#     name='../'+geom+'/larom/temp'+str(temp)+'/'+str(pressList[i])+'mpa/zrun[1-2]/out_sub*'
#     all_files=glob.glob(name)
#     for fn in all_files:
#         df=pd.read_csv(fn)
#         # print(list(df))
# #plot top figures
#         ax2a.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'min_wall_dislocations'],\
#                 '-', color='magenta', alpha=.15, linewidth=1)
#         ax2a.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'avg_wall_dislocations'],\
#                 '-', color='green', alpha=.15, linewidth=1)
#         ax2a.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'max_wall_dislocations'],\
#                 '-', color='orange', alpha=.15, linewidth=1)
# #plot bottom figures
#         ax2b.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'min_cell_dislocations'],\
#                 '-', color='magenta', alpha=.15, linewidth=1)
#         ax2b.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'avg_cell_dislocations'],\
#                 '-', color='green', alpha=.15, linewidth=1)
#         ax2b.semilogy(df.loc[::nskip,'time']/3600/24/365,\
#                 df.loc[::nskip,'max_cell_dislocations'],\
#                 '-', color='orange', alpha=.15, linewidth=1)
# #format top figures
#     if i==0:
#         ax2a.set_ylabel('wall dd (m-3)')
#     else:
#         ax2a.set_yticklabels([])
#     ax2a.set_xticklabels([])
#     ax2a.set_ylim(2e9, 3e13)
#     ax2a.set_xlim(0, time_max)
# #format bottom figures
#     if i==0:
#         ax2b.set_ylabel('cell dd (m-3)')
#     else:
#         ax2b.set_yticklabels([])
#     if i==1:
#         patch1 = mpatches.Patch(color='magenta', label='min')
#         patch2 = mpatches.Patch(color='green', label='avg')
#         patch3 = mpatches.Patch(color='orange', label='max')
#         plot_handles=[patch1, patch2, patch3]
#         ax2b.legend(handles=plot_handles,ncol=3,frameon=False,handletextpad=0.2,columnspacing=1.4,loc='upper center')
#     ax2b.set_ylim(2e9, 3e13)
#     ax2b.set_xlim(0, time_max)
#     ax2b.set_xlabel('Time (years)')
# #
#     print('done with '+str(pressList[i])+'mpa')
#
# name='allDislDens_'+geom+'_'+str(temp)+'C.pdf'
# fig2.savefig(name, format='pdf', bbox_inches='tight', pad_inches=0.1)
# #
# #----------------------------------
# #
# fig1 = plt.figure(figsize=(7.25, 3))
# ax1a = fig1.add_subplot(2,2,1)
# ax1b = fig1.add_subplot(2,2,2)
# ax1c = fig1.add_subplot(2,2,3)
# ax1d = fig1.add_subplot(2,2,4)
#
# stress_name = 'pvm_e431'#'max_vonmises_stress' #'pvm_e431qp0'
# strain_name = 'pE_e431' #'max_principal_strain' #'pE_e431qp0'
#
# nskip=5 #10
# ncut=[20*3600*24*365,19.6*3600*24*365,19.4*3600*24*365]
# eMax=0
# for i in range(3):
#     name='../'+geom+'/neml/temp'+str(temp)+'/'+str(pressList[i])+'mpa/zrun[1]/out_sub*'
#     all_files=glob.glob(name)
#     for fn in all_files:
#         df=pd.read_csv(fn)
#         df_time=df.loc[df.loc[:,'time'] < ncut[i],'time']
#         df_data=df.loc[df.loc[:,'time'] < ncut[i],strain_name]
#         ax1a.plot(df_time.iloc[::-nskip]/3600/24/365,\
#                 df_data.iloc[::-nskip],\
#                 '-', color=lineColor[i], alpha=.15, linewidth=1)
#         df_data=df.loc[df.loc[:,'time'] < ncut[i],stress_name]
#         ax1b.plot(df_time.iloc[::-nskip]/3600/24/365,\
#                 df_data.iloc[::-nskip],\
#                 '-', color=lineColor[i], alpha=.15, linewidth=1)
#         if df.loc[:,strain_name].max()*1.1>eMax:
#             eMax=df.loc[:,strain_name].max()*1.1
#     print('done with '+str(pressList[i])+'mpa')
#
# nskip=50 # 50 #points to skip in plot to reduce figure size
# plot_handles, labels= ax1c.get_legend_handles_labels()
# for i in range(3):
#     plot_handles.append(mpatches.Patch(color=lineColor[i], label=str(pressList[i])+'MPa'))
#     name='../'+geom+'/larom/temp'+str(temp)+'/'+str(pressList[i])+'mpa/zrun[1]/out_sub*'
#     all_files=glob.glob(name)
#     for fn in all_files:
#         df=pd.read_csv(fn)
#         ax1c.plot(df.loc[::-nskip,'time']/3600/24/365,\
#                 df.loc[::-nskip,strain_name],\
#                 '-', color=lineColor[i], alpha=.15, linewidth=1)
#         ax1d.plot(df.loc[::-nskip,'time']/3600/24/365,\
#                  df.loc[::-nskip,stress_name]/1e6,\
#                  '-', color=lineColor[i], alpha=.15, linewidth=1)
#     print('done with '+str(pressList[i])+'mpa')
#
# ax1a.set_ylabel('NEML\nMax Principal Strain')
# ax1a.set_ylim(0, eMax)
# ax1a.set_xlim(0, time_max)
# ax1a.set_xticklabels([])
#
# ax1b.set_ylabel('Max VM Stress (MPa)')
# ax1b.set_ylim(0.1e2, vm_max)
# ax1b.set_xlim(0, time_max)
# ax1b.set_xticklabels([])
#
# ax1c.set_ylabel('ROM\nMax Principal Strain')
# ax1c.set_ylim(0, strain_max)
# ax1c.set_xlim(0, time_max)
# ax1c.set_xlabel('Time (years)')
# ax1c.legend(handles=plot_handles,ncol=3,frameon=False,handletextpad=0.2,columnspacing=1.4,loc='upper left')
#
# ax1d.set_ylabel('Max VM Stress (MPa)')
# ax1d.set_ylim(0.1e2, vm_max)
# ax1d.set_xlim(0, time_max)
# ax1d.set_xlabel('Time (years)')
#
# fig1.subplots_adjust(wspace=.3)
# name='allVMStressHistories_'+geom+'_'+str(temp)+'C.pdf'
# fig1.savefig(name, format='pdf', bbox_inches='tight', pad_inches=0.1)
#
# plt.show()
