# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
SC = np.genfromtxt("FFM-2B_out.csv", skip_header=2, delimiter=',')
############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################
T_in = T_in = 589.15
Channels = np.array([1, 2, 4, 6, 10, 35])
EXP = np.array([15.96, 13.86, 14.7, 9.24, 14.7, 15.12])

plt.figure()
plt.plot(SC[1:] - T_in, "k^", markerfacecolor="k", label = "SubChannel")
plt.plot(EXP, "ko", markerfacecolor="r", label = "EXP")
plt.title(r"Temperature measurements 76mm downstream of heated section" "\n" "FFM-2A Run 719", fontsize=13)
plt.xticks([i for i in range(len(Channels[:]))], [str(int(i)) for i in Channels[:]])
plt.xlabel(r'$Channel~#$', fontsize=14)
plt.ylabel(r'$T-T_{in}~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("FFM-2B.png")

# %%
