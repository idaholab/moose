# %%
##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
EXP_70 = np.genfromtxt('70_percent_blockage_exp.csv',skip_header=0, delimiter=',')
EXP_90 = np.genfromtxt('90_percent_blockage_exp.csv',skip_header=0, delimiter=',')
SC = np.genfromtxt("FFM-3A_out.csv", skip_header=2, delimiter=',')
############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################
T_in_F = 826
T_in = 714.261
Channels = np.array([37, 36, 20, 10, 4, 1, 14, 28])
EXP = np.array([])

DF = np.array([118.05, 115.20, 149.14, 200.84, 235.03, 233.70, 174.23, 135.85])
F = DF + T_in_F

K = np.array([779.85, 778.26111, 797.116667, 825.8388889, 844.8333333, 844.094444, 811.0555556, 789.733333])
DK = K - T_in

print(F)

plt.figure()
plt.plot(SC[1:] - T_in, "k^", markerfacecolor="k", label = "Pronghorn-SC")
plt.plot(DK, "ko", markerfacecolor="r", label = "EXP")
plt.title(r"Temperature profile 76mm downstream of heated section" "\n" "FFM-3A Run 101", fontsize=13)
plt.xticks([i for i in range(len(Channels[:]))], [str(int(i)) for i in Channels[:]])
plt.xlabel(r'$Channel~#$', fontsize=14)
plt.ylabel(r'$T-T_{in}~[K]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.grid()
plt.savefig("FFM-3A.png")

# %%
