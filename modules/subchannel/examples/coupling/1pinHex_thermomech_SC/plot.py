# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
SC = np.genfromtxt("SC.csv",
                   skip_header=1, delimiter=',')
BISON = np.genfromtxt("BISON.csv",
                      skip_header=1, delimiter=',')
############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

plt.figure()
plt.plot(SC[:, -1], SC[:, 6], "k", label="Pin surface temp (SC) [K]")
plt.plot(BISON[:, 12], BISON[:, 0], "r:",
         label="Pin surface temp (BISON) [K]")
plt.title(r"Tpin", fontsize=13)
plt.xlabel(r'$L~[m]$', fontsize=14)
plt.ylabel(r'$Tpin~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("Tpin.png")

# plt.figure()
# plt.plot(SC[:, -1], SC[:, 10], "k",
#          label="Pin surface linear heat rate (SC) [W/m]")
# plt.plot(SC_BISON[:, -1], SC_BISON[:, 10], "r:",
#          label="Pin surface linear heat rate (SC-BISON) [W/m]")
# plt.title(r"Qprime", fontsize=13)
# plt.xlabel(r'$L~[m]$', fontsize=14)
# plt.ylabel(r'$Qprime [W/m]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.grid()
# plt.savefig("Tpin.png")
