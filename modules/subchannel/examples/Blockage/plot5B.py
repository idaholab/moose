import numpy as np
import matplotlib.pyplot as plt

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

EXP_high = np.genfromtxt("T_EXP_HIGH.csv", skip_header=0, delimiter=',')
EXP_low = np.genfromtxt("T_EXP_LOW.csv", skip_header=0, delimiter=',')
SC = np.genfromtxt("FFM-5B_out.csv", skip_header=2, delimiter=',') - 273.15

# plt.figure()
# plt.plot(SC[1:], "k^", label = "Pronghorn-SC")
# plt.plot(EXP_high[:, 1], "ko", markerfacecolor="r", label = "EXP")
# plt.title(r"Temperature profile at thermocouple location" "\n" "High flow Exp, FFM Series 6, Test 12, Run 101", fontsize=13)
# plt.xticks([i for i in range(len(EXP_high[:, 0]))],
#            [str(int(i)) for i in EXP_high[:, 0]])
# plt.xlabel(r'$Channel~\#$', fontsize=14)
# plt.ylabel(r'$T~[C]$', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.ylim(360,410)
# plt.grid()
# plt.savefig("FFM-5B.png")

plt.figure()
plt.plot(SC[1:], "k^", label = "Pronghorn-SC")
plt.plot(EXP_low[:, 1], "ko", markerfacecolor="r", label = "EXP")
plt.title(r"Temperature profile at thermocouple location" "\n" "Low flow Exp, FFM Series 6, Test 12, Run 109", fontsize=13)
plt.xticks([i for i in range(len(EXP_low[:, 0]))],
           [str(int(i)) for i in EXP_low[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[C]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(500,580)
plt.grid()
plt.savefig("FFM-5B2.png")
