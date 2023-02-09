# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
scData = np.genfromtxt("subchannel_out.csv", skip_header=0, delimiter=',')
EXP = np.genfromtxt("TTC-31_EXP.csv", skip_header=0, delimiter=',')
NETFLOW = np.genfromtxt("TTC-31_NETFLOW.csv", skip_header=0, delimiter=',')
massflow = np.genfromtxt("massflow.csv", skip_header=0, delimiter=',')
power = np.genfromtxt("power_history.csv", skip_header=0, delimiter=',')

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

plt.figure()
plt.plot(scData[7:, 0], scData[7:, 1], "k",
         label="Subchannel temperature at TTC-31")
plt.plot(EXP[:, 0], EXP[:, 1], ":r", label="Measurement at TTC-31")
plt.plot(NETFLOW[:, 0], NETFLOW[:, 1], ":g", label="NETFLOW++ at TTC-31")
plt.title(r"Transient temperature and massflow" "\n" "XX09 SHRT-17", fontsize=13)
plt.xlabel(r'$time~[sec]$', fontsize=14)
plt.ylabel(r'$Temperature~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
ax2 = plt.twinx()
# make a plot with different y-axis using second axis object
ax2.plot(massflow[:, 0], massflow[:, 1]/2.450,
         "b", label="normalized massflow")
ax2.legend(fontsize=12, loc='center right')
ax2.set_ylabel("Massflow [-]", color="blue", fontsize=14)
plt.show()
plt.savefig("Transient_Temperature2.png")

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

plt.figure()
plt.plot(power[:, 0], power[:, 1], "k", label="normalized power")
plt.title(r"Transient power of assembly" "\n" "SHRT-17", fontsize=13)
plt.xlabel(r'$time~[sec]$', fontsize=14)
plt.ylabel(r'$Power~[-]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("Transient_Power.png")


# %%
