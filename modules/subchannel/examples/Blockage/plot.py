import numpy as np
import matplotlib.pyplot as plt

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

EXP = np.genfromtxt("T_EXP_C.csv", skip_header=0, delimiter=',')
SC = np.genfromtxt("FFM-5B_out.csv", skip_header=2, delimiter=',') - 273.15

plt.figure()
plt.plot(EXP[:, 1], "r", marker='D',
         markerfacecolor="r", label="EXP")
plt.plot(SC[1:], "b", marker='D',
         markerfacecolor="b", label="Subchannel")
plt.title(r"Temperature profile at thermocouple location" "\n" "High flow Exp, FFM Series 6, Test 12, Run 101", fontsize=13)
plt.xticks([i for i in range(len(EXP[:, 0]))],
           [str(int(i)) for i in EXP[:, 0]])
plt.xlabel(r'$Channel~\#$', fontsize=14)
plt.ylabel(r'$T~[C]$', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(360,410)
plt.grid()
plt.savefig("FFM-5B_Temp.png")
