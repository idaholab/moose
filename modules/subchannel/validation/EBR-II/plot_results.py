# %%
# This script plots the validation results based on the SCM simulation
# and experimental measurements.
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
EXP = np.genfromtxt("TTC-31_EXP.csv", skip_header=1, delimiter=',')
EXP45 = np.genfromtxt("TTC-31_EXP45.csv", skip_header=1, delimiter=',')
TTC_EXP = np.genfromtxt("TTC_EXP.csv", skip_header=1, delimiter=',')
TTC_EXP45 = np.genfromtxt("TTC_EXP45.csv", skip_header=1, delimiter=',')
TTC_DASSH = np.genfromtxt("TTC_DASSH.csv", skip_header=1, delimiter=',')
NETFLOW = np.genfromtxt("TTC-31_NETFLOW.csv", skip_header=1, delimiter=',')
NETFLOW45 = np.genfromtxt("TTC-31_NETFLOW45.csv", skip_header=1, delimiter=',')
TTC31_TR17 = np.genfromtxt("XX09_SCM_TR17_out.csv", skip_header=2, delimiter=',')
TTC31_TR45R = np.genfromtxt("XX09_SCM_TR45R_out.csv", skip_header=2, delimiter=',')
TTC17 = np.genfromtxt("XX09_SCM_SS17_out.csv", skip_header=2, delimiter=',')
TTC17_corrected = np.genfromtxt("XX09_SCM_SS17_corrected_out.csv", skip_header=2, delimiter=',')
TTC45R = np.genfromtxt("XX09_SCM_SS45R_out.csv", skip_header=2, delimiter=',')
TTC45R_corrected = np.genfromtxt("XX09_SCM_SS45R_corrected_out.csv", skip_header=2, delimiter=',')
massflow_SHRT17 = np.genfromtxt(
    "massflow_SHRT17.csv", skip_header=1, delimiter=',')
massflow_SHRT45 = np.genfromtxt(
    "massflow_SHRT45.csv", skip_header=1, delimiter=',')
power_SHRT17 = np.genfromtxt(
    "power_history_SHRT17.csv", skip_header=0, delimiter=',')
power_SHRT45 = np.genfromtxt(
    "power_history_SHRT45.csv", skip_header=0, delimiter=',')

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

inlet_temperature = 624.70556
A28 = 7.3898e5
A29 = 3.154e5
A30 = 1.1340e3
A31 = -2.2153e-1
A32 = 1.1156e-4
dt = 2503.3 - inlet_temperature
cp = A28 / dt / dt + A29 / dt + A30 + A31 * dt + A32 * dt * dt
print(cp)

A48 = 1.1045e2
A49 = -6.5112e-2
A50 = 1.5430e-5
A51 = -2.4617e-9
k = A48 + A49 * inlet_temperature + A50 * inlet_temperature * inlet_temperature +\
    A51 * inlet_temperature * inlet_temperature * inlet_temperature
print(k)

A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = (A12 + A13 * inlet_temperature + A14 *
       inlet_temperature * inlet_temperature)
print(rho)

plt.figure()
plt.plot(TTC31_TR17[6:, 0], TTC31_TR17[6:, 1], "g-.",
         label="SCM caclulation at TTC-31")
plt.plot(EXP[:, 0], EXP[:, 1], marker='o', markerfacecolor="r", linestyle='none', label="Measurement at TTC-31")
plt.plot(NETFLOW[:, 0], NETFLOW[:, 1], ":b",
         label="NETFLOW++/COBRA-IV-I at TTC-31")
plt.title(r"Transient of temperature" "\n" "Assembly XX09, SHRT-17", fontsize=13)
plt.xlabel(r'$time~[sec]$', fontsize=14)
plt.ylabel(r'$Temperature~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
# plt.savefig("Transient_Temperature.png")
plt.show()

plt.figure()
plt.plot(TTC31_TR45R[7:, 0], TTC31_TR45R[7:, 1], "g-.",
         label="SCM caclulation at TTC-31")
plt.plot(EXP45[:, 0], EXP45[:, 1], marker='o', markerfacecolor="r", linestyle='none', label="Measurement at TTC-31")
plt.plot(NETFLOW45[:, 0], NETFLOW45[:, 1], ":b",
         label="NETFLOW++/COBRA-IV-I at TTC-31")
plt.title(r"Transient of temperature" "\n" "Assembly XX09, SHRT-45R", fontsize=13)
plt.xlabel(r'$time~[sec]$', fontsize=14)
plt.ylabel(r'$Temperature~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
# plt.savefig("Transient_Temperature45.png")
plt.show()

plt.figure()
plt.plot(massflow_SHRT17[:, 0], massflow_SHRT17[:, 1] /
         2.450, "k--", label="normalized massflow SHRT-17")
plt.plot(massflow_SHRT45[:, 0], massflow_SHRT45[:, 1] /
         2.427, "r-.", label="normalized massflow SHRT-45R")
plt.plot(power_SHRT17[:, 0], power_SHRT17[:, 1],
         "k", label="normalized power SHRT-17")
plt.plot(power_SHRT45[:, 0], power_SHRT45[:, 1],
         "r", label="normalized power SHRT-45R")
plt.title(
    r"Transient of normalized massflow & power" "\n" "Assembly XX09", fontsize=13)
plt.xlabel(r'$time~[sec]$', fontsize=14)
plt.ylabel(r'$[-]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
# plt.savefig("Normalized_Transients.png")
plt.show()


plt.figure()
plt.plot(TTC_EXP[:, 1] + 273.15, "r", marker='o', markerfacecolor="r", linestyle='none', label="EXP")
plt.plot(TTC_DASSH[:, 1] + 273.15, "b-.", marker='o',
         markerfacecolor="b", label="DASSH")
plt.plot(TTC17[1:], "k-.", marker='o',
         markerfacecolor="k", label="SCM")
plt.plot(TTC17_corrected[1:], "g-.", marker='o',
         markerfacecolor="g", label="SCM power profile corrected")
plt.title(r"Temperature profile 0.322m downstream of heated section" "\n" "XX09 TTC SHRT-17, Initial steady state", fontsize=13)
plt.xticks([i for i in range(len(TTC_EXP[:, 0]))],
           [str(int(i)) for i in TTC_EXP[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
# plt.savefig("XX09_TTC.png")
plt.show()

plt.figure()
plt.plot(TTC_EXP45[:, 1], "r", marker='o',
         markerfacecolor="r", linestyle='none', label="EXP")
plt.plot(TTC45R[1:], "k-.", marker='o',
         markerfacecolor="k", label="SCM")
plt.plot(TTC45R_corrected[1:], "g-.", marker='o',
         markerfacecolor="g", label="SCM power profile corrected")
plt.title(r"Temperature profile 0.322m downstream of heated section" "\n" "XX09 TTC SHRT-45R, Initial steady state", fontsize=13)
plt.xticks([i for i in range(len(TTC_EXP45[:, 0]))],
           [str(int(i)) for i in TTC_EXP45[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
# plt.savefig("XX09_TTC45.png")
plt.show()


# %%
