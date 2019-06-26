#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def powerfit(x, y, xnew):
    """line fitting on log-log scale, y=ax^n.  Returns (a, n, y(xnew))"""
    n, a = np.polyfit(np.log(np.asarray(x)), np.log(np.asarray(y)), 1)
    return np.exp(a), n, np.exp(a) * xnew**(n)




fig_num = 0

# memory, framework with nx=100
f = open("framework.log", "r")
framework_100_mem = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("Framework results for nx=100"):
        capturing_results = True
    elif line.startswith("Framework results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 1.8, dt = 0.6"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("|   1.800000e+00 |"):
        mem_per_proc = float(line.split("|")[2])
        vmem_per_proc = float(line.split("|")[3])
        framework_100_mem[procs] = [mem_per_proc, vmem_per_proc]
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [framework_100_mem[p][0] for p in sorted_procs], label = 'Physical mem per proc')
plt.plot(sorted_procs, [framework_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc')
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("Framework simulation, 100 elements")
plt.savefig("framework_100_mem.png")


# memory, framework with nx=256000
f = open("framework.log", "r")
framework_256000_mem = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("Framework results for nx=256000"):
        capturing_results = True
    elif line.startswith("Framework results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 1.8, dt = 0.6"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("|   1.800000e+00 |"):
        mem_per_proc = float(line.split("|")[2])
        vmem_per_proc = float(line.split("|")[3])
        framework_256000_mem[procs] = [mem_per_proc, vmem_per_proc]
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [framework_256000_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (raw)')
plt.plot(sorted_procs, [framework_256000_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (raw)')
plt.plot(sorted_procs, [framework_256000_mem[p][0] - framework_100_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (minus executable)')
plt.plot(sorted_procs, [framework_256000_mem[p][1] - framework_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (minus executable)')
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("Framework simulation, 256000 elements")
plt.savefig("framework_256000_mem.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [framework_256000_mem[p][0] - framework_100_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (minus executable)')
plt.loglog(sorted_procs, [framework_256000_mem[p][1] - framework_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (no executable)')
fitted = powerfit(sorted_procs, [framework_256000_mem[p][1] - framework_100_mem[p][1] for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("Framework simulation, 256000 elements")
plt.savefig("framework_256000_mem_log.png")


# cpu time, first time step, framework
f = open("framework.log", "r")
framework_1_cpu = {}
sorted_procs = []
capturing_results = False
found_timestep_1 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("Framework results for nx=256000"):
        capturing_results = True
    elif line.startswith("Framework results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_1 = False
    elif capturing_results and line.startswith("Time Step 1, time = 0.6, dt = 0.6"):
        found_timestep_1 = True
    elif capturing_results and found_timestep_1 and line.startswith("| PorousFlowTestApp (main)"):
        framework_1_cpu[procs] = float(line.split("|")[9])
        found_timestep_1 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [framework_1_cpu[p] for p in sorted_procs])
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("Framework simulation, first time step")
plt.savefig("framework_1_cpu.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [framework_1_cpu[p] for p in sorted_procs], label = 'MOOSE')
fitted = powerfit(sorted_procs, [framework_1_cpu[p] for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("Framework simulation, first time step")
plt.savefig("framework_1_cpu_log.png")

# cpu time, first time step, framework, varying number of elements
f = open("framework.log", "r")
framework_1_cpu_eles = {}
sorted_eles = []
capturing_results = False
found_timestep_1 = False
eles = 0
for line in f:
    line = line.strip()
    if line.startswith("Framework results for nx="):
        eles = int(line.split("=")[1])
        sorted_eles.append(eles)
    elif line.startswith("Processors="):
        if line == "Processors=1":
            capturing_results = True
        else:
            capturing_results = False
    elif capturing_results and line.startswith("Time Step 1, time = 0.6, dt = 0.6"):
        found_timestep_1 = True
    elif capturing_results and found_timestep_1 and line.startswith("| PorousFlowTestApp (main)"):
        framework_1_cpu_eles[eles] = float(line.split("|")[9])
        found_timestep_1 = False
f.close()
sorted_eles = sorted(sorted_eles)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_eles, [framework_1_cpu_eles[e] for e in sorted_eles])
plt.xlabel("Number of elements")
plt.ylabel("CPU time (s)")
plt.title("Framework simulation, 1 processor, first time step")
plt.savefig("framework_1_cpu_eles.png")

# cpu time, third time step, framework
f = open("framework.log", "r")
framework_3_cpu = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("Framework results for nx=256000"):
        capturing_results = True
    elif line.startswith("Framework results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 1.8, dt = 0.6"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("| PorousFlowTestApp (main)"):
        framework_3_cpu[procs] = float(line.split("|")[9])
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [0.5 * (framework_3_cpu[p] - framework_1_cpu[p]) for p in sorted_procs])
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("Framework simulation, subsequent time steps")
plt.savefig("framework_3_cpu.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [0.5 * (framework_3_cpu[p] - framework_1_cpu[p]) for p in sorted_procs], label = 'MOOSE')
fitted = powerfit(sorted_procs, [0.5 * (framework_3_cpu[p] - framework_1_cpu[p]) for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("Framework simulation, subsequent time steps")
plt.savefig("framework_3_cpu_log.png")


# memory, pffltvd_action with nx=100
f = open("pffltvd_action.log", "r")
pffltvd_action_100_mem = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("pffltvd_action results for nx=100"): # change this!!!
        capturing_results = True
    elif line.startswith("pffltvd_action results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 0.18, dt = 0.06"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("|   1.800000e-01 |"):
        mem_per_proc = float(line.split("|")[2])
        vmem_per_proc = float(line.split("|")[3])
        pffltvd_action_100_mem[procs] = [mem_per_proc, vmem_per_proc]
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [pffltvd_action_100_mem[p][0] for p in sorted_procs], label = 'Physical mem per proc')
plt.plot(sorted_procs, [pffltvd_action_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc')
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("1D Kuzmin-Turek convection, 100 elements")
plt.savefig("pffltvd_action_100_mem.png")

# memory, pffltvd_action with nx=256000
f = open("pffltvd_action.log", "r")
pffltvd_action_256000_mem = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("pffltvd_action results for nx=256000"):
        capturing_results = True
    elif line.startswith("pffltvd_action results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 0.18, dt = 0.06"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("|   1.800000e-01 |"):
        mem_per_proc = float(line.split("|")[2])
        vmem_per_proc = float(line.split("|")[3])
        pffltvd_action_256000_mem[procs] = [mem_per_proc, vmem_per_proc]
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [pffltvd_action_256000_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (raw)')
plt.plot(sorted_procs, [pffltvd_action_256000_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (raw)')
plt.plot(sorted_procs, [pffltvd_action_256000_mem[p][0] - pffltvd_action_100_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (minus executable)')
plt.plot(sorted_procs, [pffltvd_action_256000_mem[p][1] - pffltvd_action_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (minus executable)')
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("1D Kuzmin-Turek convection, 256000 elements")
plt.savefig("pffltvd_action_256000_mem.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [pffltvd_action_256000_mem[p][0] - pffltvd_action_100_mem[p][0] for p in sorted_procs], linewidth=3.0, label = 'Physical mem per proc (minus executable)')
plt.loglog(sorted_procs, [pffltvd_action_256000_mem[p][1] - pffltvd_action_100_mem[p][1] for p in sorted_procs], label = 'Virtual mem per proc (no executable)')
fitted = powerfit(sorted_procs, [pffltvd_action_256000_mem[p][1] - pffltvd_action_100_mem[p][1] for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("Memory used (MiB)")
plt.title("1D Kuzmin-Turek convection, 256000 elements")
plt.savefig("pffltvd_action_256000_mem_log.png")


# cpu time, first time step, pffltvd_action
f = open("pffltvd_action.log", "r")
pffltvd_action_1_cpu = {}
sorted_procs = []
capturing_results = False
found_timestep_1 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("pffltvd_action results for nx=256000"):
        capturing_results = True
    elif line.startswith("pffltvd_action results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_1 = False
    elif capturing_results and line.startswith("Time Step 1, time = 0.06, dt = 0.06"):
        found_timestep_1 = True
    elif capturing_results and found_timestep_1 and line.startswith("| PorousFlowTestApp (main)"):
        pffltvd_action_1_cpu[procs] = float(line.split("|")[9])
        found_timestep_1 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [pffltvd_action_1_cpu[p] for p in sorted_procs])
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("1D Kuzmin-Turek simulation, first time step")
plt.savefig("pffltvd_action_1_cpu.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [pffltvd_action_1_cpu[p] for p in sorted_procs], label = 'MOOSE')
fitted = powerfit(sorted_procs, [pffltvd_action_1_cpu[p] for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("1D Kuzmin-Turek simulation, first time step")
plt.savefig("pffltvd_action_1_cpu_log.png")


# cpu time, first time step, pffltvd_action, varying number of elements
f = open("pffltvd_action.log", "r")
pffltvd_action_1_cpu_eles = {}
sorted_eles = []
capturing_results = False
found_timestep_1 = False
eles = 0
for line in f:
    line = line.strip()
    if line.startswith("pffltvd_action results for nx="):
        eles = int(line.split("=")[1])
        sorted_eles.append(eles)
    elif line.startswith("Processors="):
        if line == "Processors=1":
            capturing_results = True
        else:
            capturing_results = False
    elif capturing_results and line.startswith("Time Step 1, time = 0.06, dt = 0.06"):
        found_timestep_1 = True
    elif capturing_results and found_timestep_1 and line.startswith("| PorousFlowTestApp (main)"):
        pffltvd_action_1_cpu_eles[eles] = float(line.split("|")[9])
        found_timestep_1 = False
f.close()
sorted_eles = sorted(sorted_eles)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_eles, [pffltvd_action_1_cpu_eles[e] for e in sorted_eles])
plt.xlabel("Number of elements")
plt.ylabel("CPU time (s)")
plt.title("1D Kuzmin-Turek simulation, first time step")
plt.savefig("pffltvd_action_1_cpu_eles.png")


# cpu time, third time step, pffltvd_action
f = open("pffltvd_action.log", "r")
pffltvd_action_3_cpu = {}
sorted_procs = []
capturing_results = False
found_timestep_3 = False
procs = 0
for line in f:
    line = line.strip()
    if line.startswith("pffltvd_action results for nx=256000"):
        capturing_results = True
    elif line.startswith("pffltvd_action results for"):
        capturing_results = False
    elif capturing_results and line.startswith("Processors="):
        procs = int(line.split("=")[1])
        sorted_procs.append(procs)
        found_timestep_3 = False
    elif capturing_results and line.startswith("Time Step 3, time = 0.18, dt = 0.06"):
        found_timestep_3 = True
    elif capturing_results and found_timestep_3 and line.startswith("| PorousFlowTestApp (main)"):
        pffltvd_action_3_cpu[procs] = float(line.split("|")[9])
        found_timestep_3 = False
f.close()
sorted_procs = sorted(sorted_procs)
fig_num += 1
plt.figure(fig_num)
plt.plot(sorted_procs, [0.5 * (pffltvd_action_3_cpu[p] - pffltvd_action_1_cpu[p]) for p in sorted_procs])
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("1D Kuzmin-Turek simulation, subsequent time steps")
plt.savefig("pffltvd_action_3_cpu.png")
fig_num += 1
plt.figure(fig_num)
plt.loglog(sorted_procs, [0.5 * (pffltvd_action_3_cpu[p] - pffltvd_action_1_cpu[p]) for p in sorted_procs], label = 'MOOSE')
fitted = powerfit(sorted_procs, [0.5 * (pffltvd_action_3_cpu[p] - pffltvd_action_1_cpu[p]) for p in sorted_procs], sorted_procs)
plt.loglog(sorted_procs, fitted[2], linestyle = ':', linewidth = 5, label = "{0:.2g}".format(fitted[0]) + "*p^" + "{0:.2g}".format(fitted[1]))
plt.legend()
plt.xlabel("Number of processors")
plt.ylabel("CPU time (s)")
plt.title("1D Kuzmin-Turek simulation, subsequent time steps")
plt.savefig("pffltvd_action_3_cpu_log.png")


sys.exit(0)

