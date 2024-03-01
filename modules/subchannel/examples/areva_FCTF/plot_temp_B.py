import numpy as np
import matplotlib.pyplot as plt

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

EXP_B4 = np.genfromtxt("EXP_temp_B4.csv", skip_header=0, delimiter=',')
EXP_BE = np.genfromtxt("EXP_temp_BE.csv", skip_header=0, delimiter=',')
SC_DATA = np.genfromtxt("FCTF_deformed_out.csv", skip_header=2, delimiter=',')
Mean_TEMP = SC_DATA[10]
SC_TEMP_B4 = SC_DATA[11:-4]
SC_TEMP_BE = SC_DATA[-4:]
Pressure_SC = SC_DATA[1:10]
Pressure_SC = np.sort(Pressure_SC)[::-1]
Pressure_SC *= 0.001
DP7 = Pressure_SC[7] - Pressure_SC[8]
DP6 = Pressure_SC[6] - Pressure_SC[7]
DP5 = Pressure_SC[5] - Pressure_SC[6]
DP4 = Pressure_SC[4] - Pressure_SC[5]
DP3 = Pressure_SC[3] - Pressure_SC[4]
DP2 = Pressure_SC[1] - Pressure_SC[2]
DP1 = Pressure_SC[0] - Pressure_SC[1]
Mean = (np.sum(SC_TEMP_B4 ) + np.sum(SC_TEMP_BE))/(len(SC_TEMP_B4) + len(SC_TEMP_BE))
Total_DP = Pressure_SC[0] - Pressure_SC[-1]
# Define the error (assuming it's the same for all points)
error = 2

### Figure with temperatures at pin surfaces at plane B
plt.figure()
# Plot the data points without error bars
plt.plot(EXP_B4[:, 0], EXP_B4[:, 1], "ks", markerfacecolor= 'None', label="EXP 4 o'clock")
plt.plot(EXP_B4[:, 0], SC_TEMP_B4 - Mean_TEMP, "rx", label="SC")

# Plot the error bars with dashed linestyle
plt.errorbar(EXP_B4[:, 0], EXP_B4[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')

plt.title(r"Temperature measurements at plane B", fontsize=13)
plt.xlabel(r'$Pin~Number~\#$', fontsize=14)
plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean~[C]', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(-10, 20)

# Set major ticks at every 5 units
major_ticks = range(int(min(EXP_B4[:, 0])), int(max(EXP_B4[:, 0])) + 1, 5)
plt.xticks(major_ticks)

# Set minor ticks at every 1 unit
minor_ticks = range(int(min(EXP_B4[:, 0])), int(max(EXP_B4[:, 0])) + 1, 1)
plt.xticks(minor_ticks, minor=True)

# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)

plt.savefig("TEMP-B4.png")
plt.show()
######################

#### Figure with temperatures at face E and plane B
plt.figure()
# Plot the data points without error bars
plt.plot(EXP_BE[:, 0], EXP_BE[:, 1], "ks", markerfacecolor= 'None', label="EXP temperature at wall E")
plt.plot(EXP_BE[:, 0], SC_TEMP_BE - Mean_TEMP, "rx", label="SC")
# Plot the error bars with dashed linestyle
plt.errorbar(EXP_BE[:, 0], EXP_BE[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')

plt.title(r"Temperature measurements at plane B and Face E", fontsize=13)
plt.xlabel(r'$TC~Number~\#$', fontsize=14)
plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean~[C]', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(-6, 4)

# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)

plt.savefig("TEMP-BE.png")
plt.show()
########################################

###### Figure with pressure drops on Face B across specified axial segements
axial_heights = np.array([-4, -3, -2, -1.5, -0.5, 0.0, 3, 6, 8])
exp_dp = np.array([9267.0, 10379.6, 10174.7, 2349.1, 26700.5, 27453.8, 18820.2])
exp_dp *=0.001
## EXP
plt.figure()
for i in range(5):
    plt.plot([axial_heights[i+3],axial_heights[i+4]], [exp_dp[i+2], exp_dp[i+2]], color='black', linestyle='-', linewidth=2)

plt.plot([axial_heights[0],axial_heights[1]], [exp_dp[0], exp_dp[0]], color='black', linestyle='-', linewidth=2)
plt.plot([axial_heights[1],axial_heights[2]], [exp_dp[1], exp_dp[1]], color='black', linestyle='-', linewidth=2, label = 'EXP')
## SC
plt.plot([-4,-3], [DP1, DP1], color='red', linestyle=':', linewidth=2, label = 'SC')
plt.plot([-3,-2], [DP2, DP2], color='red', linestyle=':', linewidth=2)
plt.plot([-1.5,-0.5], [DP3, DP3], color='red', linestyle=':', linewidth=2)
plt.plot([-0.5,0.0], [DP4, DP4], color='red', linestyle=':', linewidth=2)
plt.plot([0,3], [DP5, DP5], color='red', linestyle=':', linewidth=2)
plt.plot([3,6], [DP6, DP6], color='red', linestyle=':', linewidth=2)
plt.plot([6,8], [DP7, DP7], color='red', linestyle=':', linewidth=2)

plt.title(r"Pressure Drop measurements, at face B", fontsize=13)
plt.xlabel(r'$Axial~Segment~[wire~Pitch~from~heated~entry]$', fontsize=14)
plt.ylabel(r'Pressure Drop [Pa]', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(0, 45)
plt.xlim(-4, 9)
plt.yticks(np.arange(0, 45, 5), fontsize=14)  # Set y-axis ticks every 1, starting from 0
# Initialize minor ticks
plt.minorticks_on()

# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)

plt.savefig("DP-B.png")
plt.show()
