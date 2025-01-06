# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
scData = np.genfromtxt("SC_tr_out.csv", skip_header=1, delimiter=',')
scData45 = np.genfromtxt("SC45_tr_out.csv", skip_header=1, delimiter=',')
EXP = np.genfromtxt("TTC-31_EXP.csv", skip_header=1, delimiter=',')
EXP45 = np.genfromtxt("TTC-31_EXP45.csv", skip_header=1, delimiter=',')
TTC_EXP = np.genfromtxt("TTC_EXP.csv", skip_header=1, delimiter=',')
TTC_EXP45 = np.genfromtxt("TTC_EXP45.csv", skip_header=1, delimiter=',')
TTC_DASSH = np.genfromtxt("TTC_DASSH.csv", skip_header=1, delimiter=',')
NETFLOW = np.genfromtxt("TTC-31_NETFLOW.csv", skip_header=1, delimiter=',')
NETFLOW45 = np.genfromtxt("TTC-31_NETFLOW45.csv", skip_header=1, delimiter=',')
SC_TTC = np.genfromtxt("XX09_TTC.csv", skip_header=2, delimiter=',')
TTC17_nominal = np.genfromtxt("TTC17_nominal.csv", skip_header=2, delimiter=',')
TTC31_TR17 = np.genfromtxt("TTC31_TR17.csv", skip_header=2, delimiter=',')
TTC31_TR45R = np.genfromtxt("TTC31_TR45R.csv", skip_header=2, delimiter=',')
TTC45R_nominal = np.genfromtxt("TTC45R_nominal.csv", skip_header=2, delimiter=',')
TTC17_nominal_corrected = np.genfromtxt("TTC17_nominal_corrected.csv", skip_header=2, delimiter=',')
TTC45R_nominal_corrected = np.genfromtxt("TTC45R_nominal_corrected.csv", skip_header=2, delimiter=',')
# SC_TTC45 = np.genfromtxt("XX09_TTC45.csv", skip_header=2, delimiter=',')
SC_TTC_corrected = np.genfromtxt(
    "XX09_SC_corrected.csv", skip_header=2, delimiter=',')
SC_MTC = np.genfromtxt("SC_MTC.csv", skip_header=2, delimiter=',')
SC_14TC = np.genfromtxt("SC_14TC.csv", skip_header=2, delimiter=',')
SC_14TC45 = np.genfromtxt("SC_14TC45.csv", skip_header=2, delimiter=',')
massflow_SHRT17 = np.genfromtxt(
    "massflow_SHRT17.csv", skip_header=1, delimiter=',')
massflow_SHRT45 = np.genfromtxt(
    "massflow_SHRT45.csv", skip_header=1, delimiter=',')
power_SHRT17 = np.genfromtxt(
    "power_history_SHRT17.csv", skip_header=0, delimiter=',')
power_SHRT45 = np.genfromtxt(
    "power_history_SHRT45.csv", skip_header=0, delimiter=',')
EXP_MTC = np.genfromtxt("EXP_MTC.csv", skip_header=1, delimiter=',')
DASSH_MTC = np.genfromtxt("DASSH_MTC.csv", skip_header=1, delimiter=',')
EXP_14TC = np.genfromtxt("EXP_14TC.csv", skip_header=1, delimiter=',')
# EXP_14TC45 = np.genfromtxt("EXP_14TC45.csv", skip_header=1, delimiter=',')
DASSH_14TC = np.genfromtxt("DASSH_14TC.csv", skip_header=1, delimiter=',')
coupled = np.genfromtxt("Pr_SC_SS_out_subchannel0.csv",
                        skip_header=2, delimiter=',')

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
# plt.plot(scData[6:, 0], scData[6:, 1], "k",
#          label="SC at TTC-31")
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
plt.savefig("Transient_Temperature.png")
plt.show()

plt.figure()
# plt.plot(scData45[6:, 0], scData45[6:, 1], "k",
#          label="SC at TTC-31")
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
plt.savefig("Transient_Temperature45.png")
plt.show()

# plt.figure()
# plt.plot(massflow_SHRT17[:, 0], massflow_SHRT17[:, 1] /
#          2.450, "k--", label="normalized massflow SHRT-17")
# plt.plot(massflow_SHRT45[:, 0], massflow_SHRT45[:, 1] /
#          2.427, "r-.", label="normalized massflow SHRT-45R")
# plt.plot(power_SHRT17[:, 0], power_SHRT17[:, 1],
#          "k", label="normalized power SHRT-17")
# plt.plot(power_SHRT45[:, 0], power_SHRT45[:, 1],
#          "r", label="normalized power SHRT-45R")
# plt.title(
#     r"Transient of normalized massflow & power" "\n" "Assembly XX09", fontsize=13)
# plt.xlabel(r'$time~[sec]$', fontsize=14)
# plt.ylabel(r'$[-]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("Normalized_Transients.png")
# plt.show()


# plt.figure()
# plt.plot(TTC_EXP[:, 1] + 273.15, "r", marker='o', markerfacecolor="r", linestyle='none', label="EXP")
# plt.plot(TTC_DASSH[:, 1] + 273.15, "b-.", marker='o',
#          markerfacecolor="b", label="DASSH")
# # plt.plot(SC_TTC[1:], "k", marker='D',
# #          markerfacecolor="k", label="SCM")
# plt.plot(TTC17_nominal[1:], "k-.", marker='o',
#          markerfacecolor="k", label="SCM")
# plt.plot(TTC17_nominal_corrected[1:], "g-.", marker='o',
#          markerfacecolor="g", label="SCM power profile corrected")
# # plt.plot(SC_TTC_corrected[1:], "g", marker='D',
# #          markerfacecolor="g", label="SCM with power profile corrected")
# # plt.plot(coupled[1:], "k-.", marker='D',
# #          markerfacecolor="k", label="Pr-SC power profile corrected")
# plt.title(r"Temperature profile 0.322m downstream of heated section" "\n" "XX09 TTC SHRT-17, Initial steady state", fontsize=13)
# plt.xticks([i for i in range(len(TTC_EXP[:, 0]))],
#            [str(int(i)) for i in TTC_EXP[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("XX09_TTC.png")

# plt.figure()
# plt.plot(TTC_EXP45[:, 1], "r", marker='o',
#          markerfacecolor="r", linestyle='none', label="EXP")
# plt.plot(TTC45R_nominal[1:], "k-.", marker='o',
#          markerfacecolor="k", label="SCM")
# plt.plot(TTC45R_nominal_corrected[1:], "g-.", marker='o',
#          markerfacecolor="g", label="SCM power profile corrected")
# # plt.plot(SC_TTC45_corrected[1:], "g", marker='D',
# #          markerfacecolor="g", label="SC power profile corrected")
# # plt.plot(coupled45[1:], "k-.", marker='D',
# #          markerfacecolor="k", label="Pr-SC power profile corrected")
# plt.title(r"Temperature profile 0.322m downstream of heated section" "\n" "XX09 TTC SHRT-45R, Initial steady state", fontsize=13)
# plt.xticks([i for i in range(len(TTC_EXP45[:, 0]))],
#            [str(int(i)) for i in TTC_EXP45[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("XX09_TTC45.png")


# plt.figure()
# plt.plot(EXP_MTC[:, 1] + 273.15, "r", marker='D',
#          markerfacecolor="r", label="EXP MTC")
# plt.plot(DASSH_MTC[:, 1] + 273.15, "b", marker='D',
#          markerfacecolor="b", label="DASSH MTC")
# plt.plot(SC_MTC[1:], "k", marker='D',
#          markerfacecolor="k", label="SC MTC")
# plt.title(r"Temperature profile 0.172m downstream of heated section" "\n" "XX09 SHRT-17, Initial steady state", fontsize=13)
# plt.xticks([i for i in range(len(EXP_MTC[:, 0]))],
#            [str(int(i)) for i in EXP_MTC[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("XX09_MTC.png")

# plt.figure()
# plt.plot(EXP_14TC[:, 1] + 273.15, "r", marker='D',
#          markerfacecolor="r", label="EXP 14TC")
# plt.plot(DASSH_14TC[:, 1] + 273.15, "b", marker='D',
#          markerfacecolor="b", label="DASSH 14TC")
# plt.plot(SC_14TC[1:], "k", marker='D',
#          markerfacecolor="k", label="SC 14TC")
# plt.title(r"Temperature profile 0.480m downstream of heated section" "\n" "XX09 SHRT-17, Initial steady state", fontsize=13)
# plt.xticks([i for i in range(len(EXP_14TC[:, 0]))],
#            [str(int(i)) for i in EXP_14TC[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("XX09_14TC.png")

# plt.figure()
# plt.plot(EXP_14TC45[:, 1], "r", marker='D',
#          markerfacecolor="r", label="EXP 14TC")
# plt.plot(SC_14TC45[1:], "k", marker='D',
#          markerfacecolor="k", label="SC 14TC")
# plt.title(r"Temperature profile 0.480m downstream of heated section" "\n" "XX09 SHRT-45R, Initial steady state", fontsize=13)
# plt.xticks([i for i in range(len(EXP_14TC45[:, 0]))],
#            [str(int(i)) for i in EXP_14TC45[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("XX09_14TC45.png")


# %%
