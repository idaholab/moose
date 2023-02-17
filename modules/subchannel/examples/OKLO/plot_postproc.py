# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
scData = np.genfromtxt("subchannel_out.csv", skip_header=1, delimiter=',')
EXP = np.genfromtxt("TTC-31_EXP.csv", skip_header=1, delimiter=',')
TTC_EXP = np.genfromtxt("TTC_EXP.csv", skip_header=1, delimiter=',')
TTC_DASSH = np.genfromtxt("TTC_DASSH.csv", skip_header=1, delimiter=',')
NETFLOW = np.genfromtxt("TTC-31_NETFLOW.csv", skip_header=1, delimiter=',')
SC_TTC = np.genfromtxt("XX09_TTC.csv", skip_header=2, delimiter=',')
SC_MTC = np.genfromtxt("SC_MTC.csv", skip_header=2, delimiter=',')
SC_14TC = np.genfromtxt("SC_14TC.csv", skip_header=2, delimiter=',')
massflow = np.genfromtxt("massflow.csv", skip_header=1, delimiter=',')
power = np.genfromtxt("power_history.csv", skip_header=1, delimiter=',')
EXP_MTC = np.genfromtxt("EXP_MTC.csv", skip_header=1, delimiter=',')
DASSH_MTC = np.genfromtxt("DASSH_MTC.csv", skip_header=1, delimiter=',')
EXP_14TC = np.genfromtxt("EXP_14TC.csv", skip_header=1, delimiter=',')
DASSH_14TC = np.genfromtxt("DASSH_14TC.csv", skip_header=1, delimiter=',')

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

# plt.figure()
# plt.plot(scData[7:, 0], scData[7:, 1], "k",
#          label="Subchannel temperature at TTC-31")
# plt.plot(EXP[:, 0], EXP[:, 1], ":r", label="Measurement at TTC-31")
# plt.plot(NETFLOW[:, 0], NETFLOW[:, 1], ":g", label="NETFLOW++ at TTC-31")
# plt.title(r"Transient of temperature and normalized massflow" "\n" "Assembly XX09, SHRT-17", fontsize=13)
# plt.xlabel(r'$time~[sec]$', fontsize=14)
# plt.ylabel(r'$Temperature~[K]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# ax2 = plt.twinx()
# # make a plot with different y-axis using second axis object
# ax2.plot(massflow[:, 0], massflow[:, 1]/2.450,
#          "b", label="normalized massflow")
# ax2.legend(fontsize=12, loc='center right')
# ax2.set_ylabel("Massflow [-]", color="blue", fontsize=14)
# plt.savefig("Transient_Temperature.png")
# plt.show()


# plt.figure()
# plt.plot(massflow[:, 0], massflow[:, 1]/2.450,
#          "k", label="normalized massflow")
# plt.title(r"Transient massflow XX09" "\n" "SHRT-17", fontsize=13)
# plt.xlabel(r'$time~[sec]$', fontsize=14)
# plt.ylabel(r'$Massflow~[-]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("Transient_Massflow.png")

# plt.figure()
# plt.plot(power[:, 0], power[:, 1], "k", label="normalized power")
# plt.title(r"Transient power of assembly" "\n" "SHRT-17", fontsize=13)
# plt.xlabel(r'$time~[sec]$', fontsize=14)
# plt.ylabel(r'$Power~[-]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("Transient_Power.png")


plt.figure()
plt.plot(TTC_EXP[:, 1] + 273.15, "r", marker='D',
         markerfacecolor="r", label="EXP")
plt.plot(TTC_DASSH[:, 1] + 273.15, "b", marker='D',
         markerfacecolor="b", label="DASSH")
plt.plot(SC_TTC[1:], "k", marker='D',
         markerfacecolor="k", label="Pronghorn-SC")
plt.title(r"Temperature profile 0.322m downstream of heated section" "\n" "XX09 TTC SHRT-17, Initial steady state", fontsize=13)
plt.xticks([i for i in range(len(TTC_EXP[:, 0]))],
           [str(int(i)) for i in TTC_EXP[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("XX09_TTC.png")


plt.figure()
plt.plot(EXP_MTC[:, 1] + 273.15, "r", marker='D',
         markerfacecolor="r", label="EXP MTC")
plt.plot(DASSH_MTC[:, 1] + 273.15, "b", marker='D',
         markerfacecolor="b", label="DASSH MTC")
plt.plot(SC_MTC[1:], "k", marker='D',
         markerfacecolor="k", label="Pronghorn-SC MTC")
plt.title(r"Temperature profile 0.172m downstream of heated section" "\n" "XX09 SHRT-17, Initial steady state", fontsize=13)
plt.xticks([i for i in range(len(EXP_MTC[:, 0]))],
           [str(int(i)) for i in EXP_MTC[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("XX09_MTC.png")

plt.figure()
plt.plot(EXP_14TC[:, 1] + 273.15, "r", marker='D',
         markerfacecolor="r", label="EXP 14TC")
plt.plot(DASSH_14TC[:, 1] + 273.15, "b", marker='D',
         markerfacecolor="b", label="DASSH 14TC")
plt.plot(SC_14TC[1:], "k", marker='D',
         markerfacecolor="k", label="Pronghorn-SC 14TC")
plt.title(r"Temperature profile 0.480m downstream of heated section" "\n" "XX09 SHRT-17, Initial steady state", fontsize=13)
plt.xticks([i for i in range(len(EXP_14TC[:, 0]))],
           [str(int(i)) for i in EXP_14TC[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("XX09_14TC.png")


# %%
