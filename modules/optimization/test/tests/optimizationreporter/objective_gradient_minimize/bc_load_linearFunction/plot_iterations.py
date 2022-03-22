#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import pandas as pd
import os
import sys
import numpy as np
import matplotlib.pyplot as plt
import math

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
#-------------------------------------------------------------------------------
df_nls = pd.read_csv('nls_optInfo_0001.csv')
df_cg = pd.read_csv('cg_optInfo_0001.csv')
df_nm = pd.read_csv('nm_optInfo_0001.csv')
df_lmvm = pd.read_csv('lmvm_optInfo_0001.csv')
print('fracture dataframe description: ',df_nls.columns.values)

#-------------------------------------------------------------------------------
fig = plt.figure(figsize=(7.25, 2.5))
ax = fig.add_subplot(1,2,1)
ax.semilogy(df_cg['current_iterate'],df_cg['function_value'],'b',label='cg')
ax.semilogy(df_nm['current_iterate'],df_nm['function_value'],'g',label='nm')
ax.semilogy(df_lmvm['current_iterate'],df_lmvm['function_value'],'c',label='lmvm')
ax.semilogy(df_nls['current_iterate'],df_nls['function_value'],'r',label='nls')
ax.set_ylabel("objective")
ax.set_xlabel("iteration")
ax.grid()
ax.legend()
ax.set_xlim([-1, 100])
#-------------------------------------------------------------------------------

df_nls['cumulative_forward_solves'] = df_nls['objective_iterate'].cumsum()
df_nls['cumulative_grad_solves'] = df_nls['gradient_iterate'].cumsum()
df_nls['total_solves'] = df_nls['cumulative_forward_solves'] + df_nls['cumulative_grad_solves']

df_cg['cumulative_forward_solves'] = df_cg['objective_iterate'].cumsum()
df_cg['cumulative_grad_solves'] = df_cg['gradient_iterate'].cumsum()
df_cg['total_solves'] = df_cg['cumulative_forward_solves'] + df_cg['cumulative_grad_solves']

df_nm['cumulative_forward_solves'] = df_nm['objective_iterate'].cumsum()
df_nm['cumulative_grad_solves'] = df_nm['gradient_iterate'].cumsum()
df_nm['total_solves'] = df_nm['cumulative_forward_solves'] + df_nm['cumulative_grad_solves']

df_lmvm['cumulative_forward_solves'] = df_lmvm['objective_iterate'].cumsum()
df_lmvm['cumulative_grad_solves'] = df_lmvm['gradient_iterate'].cumsum()
df_lmvm['total_solves'] = df_lmvm['cumulative_forward_solves'] + df_lmvm['cumulative_grad_solves']

ax = fig.add_subplot(1,2,2)
ax.semilogy(df_cg['total_solves'],df_cg['function_value'],'b',label='cg')
ax.semilogy(df_nm['total_solves'],df_nm['function_value'],'g',label='nm')
ax.semilogy(df_lmvm['total_solves'],df_lmvm['function_value'],'c',label='lmvm')
ax.semilogy(df_nls['total_solves'],df_nls['function_value'],'r',label='nls')
ax.set_yticklabels([])
ax.set_xlabel("total_solves")
ax.grid()
ax.set_xlim([-1, 100])


fig.savefig('convergence.pdf', format='pdf', bbox_inches='tight', pad_inches=0.1)

# plt.show()
