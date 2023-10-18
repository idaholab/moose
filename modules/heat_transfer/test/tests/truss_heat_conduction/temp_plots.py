#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Plotting the results of line.i, strip.i, rectangle_w_line.i, rectangle_w_strip.i, block_w_bar.i, and block_w_line.i

import mooseutils
import matplotlib.pyplot as plt

data_line = mooseutils.VectorPostprocessorReader('csv/line_center_*.csv')
data_strip = mooseutils.VectorPostprocessorReader('csv/strip_center_*.csv')

data_rectangle_w_line_x_025 = mooseutils.VectorPostprocessorReader('csv/rectangle_w_line_x_0_25_*.csv')
data_rectangle_w_line_x_n025 = mooseutils.VectorPostprocessorReader('csv/rectangle_w_line_x_n0_25_*.csv')

data_rectangle_w_strip_x_025 = mooseutils.VectorPostprocessorReader('csv/rectangle_w_strip_x_0_25_*.csv')
data_rectangle_w_strip_x_n025 = mooseutils.VectorPostprocessorReader('csv/rectangle_w_strip_x_n0_25_*.csv')

data_block_w_line_x_0_25 = mooseutils.VectorPostprocessorReader('csv/block_w_line_x_0_25_*.csv')
data_block_w_line_x_n0_25 = mooseutils.VectorPostprocessorReader('csv/block_w_line_x_n0_25_*.csv')

data_block_w_bar_x_0_25 = mooseutils.VectorPostprocessorReader('csv/block_w_bar_x_0_25_*.csv')
data_block_w_bar_x_n0_25 = mooseutils.VectorPostprocessorReader('csv/block_w_bar_x_n0_25_*.csv')

time = [1]

f1 = plt.figure(2, figsize=(6,4.5))
ax = plt.gca()
for step in time:
  t = step
  data_line.update(time=t)
  data_line.data.data.plot(ax=ax,x='id',y='temperature',label='line',marker='o',mfc='none',ms=2,color='black',ls='-',lw=1)
  data_strip.data.data.plot(ax=ax,x='id',y='temperature',label='strip',color='red',ls='-',lw=1)

plt.xlabel('X coordinate (m)')
plt.ylabel('Temperature ($\degree$C)')
plt.savefig('line_or_strip.pdf')
plt.close()

f2 = plt.figure(2, figsize=(6,4.5))
ax = plt.gca()
for step in time:
  t = step
  data_rectangle_w_line_x_025.update(time=t)
  data_rectangle_w_line_x_025.data.data.plot(ax=ax,x='id',y='temperature',label='Embedded line x=0.25',marker='o',mfc='none',ms=2,color='black',ls='-',lw=1)
  data_rectangle_w_line_x_n025.data.data.plot(ax=ax,x='id',y='temperature',label='Embedded line x=-0.25',marker='o',mfc='none',ms=2,color='black',ls='--',lw=1)
  data_rectangle_w_strip_x_025.data.data.plot(ax=ax,x='id',y='temperature',label='Strip x=0.25',color='red',ls='-',lw=1)
  data_rectangle_w_strip_x_n025.data.data.plot(ax=ax,x='id',y='temperature',label='Strip x=-0.25',color='red',ls='--',lw=1)
plt.xlabel('Y coordinate')
plt.ylabel('Temperature ($\degree$C)')
plt.savefig('rectangle_w_line_or_strip.pdf')
plt.close()


f3 = plt.figure(2, figsize=(6,4.5))
ax = plt.gca()
for step in time:
  t = step
  data_block_w_line_x_0_25.update(time=t)
  data_block_w_line_x_0_25.data.data.plot(ax=ax,x='id',y='temperature',label='Embedded line x=0.25',marker='o',mfc='none',ms=2,color='black',ls='-',lw=1)
  data_block_w_line_x_n0_25.data.data.plot(ax=ax,x='id',y='temperature',label='Embedded line x=-0.25',marker='o',mfc='none',ms=2,color='black',ls='--',lw=1)
  data_block_w_bar_x_0_25.data.data.plot(ax=ax,x='id',y='temperature',label='Bar x=0.25',color='red',ls='-',lw=1)
  data_block_w_bar_x_n0_25.data.data.plot(ax=ax,x='id',y='temperature',label='Bar x=-0.25',color='red',ls='--',lw=1)
plt.xlabel('Y coordinate')
plt.ylabel('Temperature ($\degree$C)')
plt.savefig('block_w_line_or_bar.pdf')
plt.close()
