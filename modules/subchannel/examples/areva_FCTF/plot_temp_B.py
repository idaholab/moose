import numpy as np
import matplotlib.pyplot as plt

############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################

### Plane B
EXP_B4 = np.genfromtxt("EXP_temp_B4.csv", skip_header=0, delimiter=',')
EXP_B6 = np.genfromtxt("EXP_temp_B6.csv", skip_header=0, delimiter=',')
EXP_B = (EXP_B4 + EXP_B6) / 2.0
EXP_BE = np.genfromtxt("EXP_temp_BE.csv", skip_header=0, delimiter=',')
###
EXP_B4_ND = np.genfromtxt("EXP_temp_B4_ND.csv", skip_header=0, delimiter=',')
EXP_B6_ND = np.genfromtxt("EXP_temp_B6_ND.csv", skip_header=0, delimiter=',')
EXP_B_ND = (EXP_B4_ND + EXP_B6_ND) / 2.0
EXP_BE_ND = np.genfromtxt("EXP_temp_BE_ND.csv", skip_header=0, delimiter=',')
#### Plane C
EXP_C8 = np.genfromtxt("EXP_temp_C8.csv", skip_header=0, delimiter=',')
EXP_C10 = np.genfromtxt("EXP_temp_C10.csv", skip_header=0, delimiter=',')
EXP_C = (EXP_C8 + EXP_C10) / 2.0
EXP_CE = np.genfromtxt("EXP_temp_CE.csv", skip_header=0, delimiter=',')
###
EXP_C8_ND = np.genfromtxt("EXP_temp_C8_ND.csv", skip_header=0, delimiter=',')
EXP_C10_ND = np.genfromtxt("EXP_temp_C10_ND.csv", skip_header=0, delimiter=',')
EXP_C_ND = (EXP_C8_ND + EXP_C10_ND) / 2.0
EXP_CE_ND = np.genfromtxt("EXP_temp_CE_ND.csv", skip_header=0, delimiter=',')

### Deformed Duct SCM Results
SC_DATA = np.genfromtxt("FCTF_deformed_out.csv", skip_header=2, delimiter=',')
SC_TEMP = SC_DATA[11:-4]
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
Mean = (np.sum(SC_TEMP ) + np.sum(SC_TEMP_BE))/(len(SC_TEMP) + len(SC_TEMP_BE))
Total_DP = Pressure_SC[0] - Pressure_SC[-1]
### Non-Deformed Duct SCM results
SC_DATA_ND = np.genfromtxt("FCTF_non_deformed_out.csv", skip_header=2, delimiter=',')
SC_TEMP_ND = SC_DATA_ND[12:-4]
SC_TEMP_BE_ND = SC_DATA_ND[-4:]
Mean_ND = (np.sum(SC_TEMP_ND ) + np.sum(SC_TEMP_BE_ND))/(len(SC_TEMP_ND) + len(SC_TEMP_BE_ND))
Pressure_SC_ND = SC_DATA_ND[1:11]
Pressure_SC_ND *= 0.001
###### Figure with pressure drops on Face B across specified axial segements
axial_heights = np.array([-4, -3, -2, -1.5, -0.5, 0.0, 3, 6, 8])
axial_heights_ND = np.array([-4, -3, -2, -1.5, -0.5, 0.0, 0.5, 3, 6, 8])
exp_dp = np.array([9267.0, 10379.6, 10174.7, 2349.1, 26700.5, 27453.8, 18820.2])
exp_dp_ND = np.array([9.091487, 10.0495, 10.12066641, 2.17931433, 9.934803544, 7.775231125, 29.20108, 18.54949923])
CFD_dp = np.array([10534.884, 10325.581, 10325.581, 1325.581, 27139.535, 27906.977, 20511.628])
CFD_dp_ND = np.array([9.85391, 9.604873, 9.6122, 1.1623652, 9.55345, 8.410632, 28.628467, 18.931])
exp_dp *=0.001
CFD_dp *=0.001
DP1_ND = Pressure_SC_ND[0] - Pressure_SC_ND[1]
DP2_ND = Pressure_SC_ND[1] - Pressure_SC_ND[2]
DP3_ND = Pressure_SC_ND[3] - Pressure_SC_ND[4]
DP4_ND = Pressure_SC_ND[4] - Pressure_SC_ND[5]
DP5_ND = Pressure_SC_ND[4] - Pressure_SC_ND[6]
DP6_ND = Pressure_SC_ND[5] - Pressure_SC_ND[6]
DP7_ND = Pressure_SC_ND[5] - Pressure_SC_ND[7]
DP8_ND = Pressure_SC_ND[8] - Pressure_SC_ND[9]

##############################
# Define the error
error = 2
# ##############################
# ## EXP Deformed Duct
# plt.figure()
# for i in range(5):
#     plt.plot([axial_heights[i+3],axial_heights[i+4]], [exp_dp[i+2], exp_dp[i+2]], color='black', linestyle='-', linewidth=2)
#     plt.plot([axial_heights[i+3],axial_heights[i+4]], [CFD_dp[i+2], CFD_dp[i+2]], color='green', linestyle=':', linewidth=2)

# plt.plot([axial_heights[0],axial_heights[1]], [exp_dp[0], exp_dp[0]], color='black', linestyle='-', linewidth=2)
# plt.plot([axial_heights[1],axial_heights[2]], [exp_dp[1], exp_dp[1]], color='black', linestyle='-', linewidth=2, label = 'EXP')
# plt.plot([axial_heights[0],axial_heights[1]], [CFD_dp[0], CFD_dp[0]], color='green', linestyle=':', linewidth=2)
# plt.plot([axial_heights[1],axial_heights[2]], [CFD_dp[1], CFD_dp[1]], color='green', linestyle=':', linewidth=2, label = 'CFD')
# ## SC
# plt.plot([-4,-3], [DP1, DP1], color='red', linestyle=':', linewidth=2, label = 'SCM')
# plt.plot([-3,-2], [DP2, DP2], color='red', linestyle=':', linewidth=2)
# plt.plot([-1.5,-0.5], [DP3, DP3], color='red', linestyle=':', linewidth=2)
# plt.plot([-0.5,0.0], [DP4, DP4], color='red', linestyle=':', linewidth=2)
# plt.plot([0,3], [DP5, DP5], color='red', linestyle=':', linewidth=2)
# plt.plot([3,6], [DP6, DP6], color='red', linestyle=':', linewidth=2)
# plt.plot([6,8], [DP7, DP7], color='red', linestyle=':', linewidth=2)

# plt.title(r'Pressure Drop across Face B''\n' 'for the deformed test bundle', fontsize=13)
# plt.xlabel(r'$Axial~Segment~[wire~Pitch~from~heated~entry]$', fontsize=14)
# plt.ylabel(r'Pressure Drop [Pa]', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.ylim(0, 35)
# plt.xlim(-4, 9)
# plt.yticks(np.arange(0, 36, 5), fontsize=14)  # Set y-axis ticks every 1, starting from 0
# plt.xticks([-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9])
# # Set grid lines
# plt.grid(which='both', linestyle='--', linewidth=0.5)
# plt.savefig("DP-FaceB.png")
# plt.show()

# ##############################
# ##############################
# ## EXP NON Deformed Duct
# plt.plot([-4,-3], [exp_dp_ND[0], exp_dp_ND[0]], color='black', linestyle='-', linewidth=2)
# plt.plot([-3,-2], [exp_dp_ND[1], exp_dp_ND[1]], color='black', linestyle='-', linewidth=2)
# plt.plot([-1.5,-0.5], [exp_dp_ND[2], exp_dp_ND[2]], color='black', linestyle='-', linewidth=2)
# plt.plot([-0.5,0.0], [exp_dp_ND[3], exp_dp_ND[3]], color='black', linestyle='-', linewidth=2)
# plt.plot([-0.5,0.5], [exp_dp_ND[4], exp_dp_ND[4]], color='black', linestyle='-', linewidth=2)
# plt.plot([0.0,0.5], [exp_dp_ND[5], exp_dp_ND[5]], color='black', linestyle='-', linewidth=2)
# plt.plot([0.0,3], [exp_dp_ND[6], exp_dp_ND[6]], color='black', linestyle='-', linewidth=2)
# plt.plot([6,8], [exp_dp_ND[7], exp_dp_ND[7]], color='black', linestyle='-', linewidth=2, label = 'EXP')
# ### CFD
# plt.plot([-4,-3], [CFD_dp_ND[0], CFD_dp_ND[0]], color='green', linestyle=':', linewidth=2)
# plt.plot([-3,-2], [CFD_dp_ND[1], CFD_dp_ND[1]], color='green', linestyle=':', linewidth=2)
# plt.plot([-1.5,-0.5], [CFD_dp_ND[2], CFD_dp_ND[2]], color='green', linestyle=':', linewidth=2)
# plt.plot([-0.5,0.0], [CFD_dp_ND[3], CFD_dp_ND[3]], color='green', linestyle=':', linewidth=2)
# plt.plot([-0.5,0.5], [CFD_dp_ND[4], CFD_dp_ND[4]], color='green', linestyle=':', linewidth=2)
# plt.plot([0.0,0.5], [CFD_dp_ND[5], CFD_dp_ND[5]], color='green', linestyle=':', linewidth=2)
# plt.plot([0.0,3], [CFD_dp_ND[6], CFD_dp_ND[6]], color='green', linestyle=':', linewidth=2)
# plt.plot([6,8], [CFD_dp_ND[7], CFD_dp_ND[7]], color='green', linestyle=':', linewidth=2, label = 'CFD')
# ### CFD
# plt.plot([-4,-3], [DP1_ND, DP1_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([-3,-2], [DP2_ND, DP2_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([-1.5,-0.5], [DP3_ND, DP3_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([-0.5,0.0], [DP4_ND, DP4_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([-0.5,0.5], [DP5_ND, DP5_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([0.0,0.5], [DP6_ND, DP6_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([0.0,3], [DP7_ND, DP7_ND], color='red', linestyle=':', linewidth=2)
# plt.plot([6,8], [DP8_ND, DP8_ND], color='red', linestyle=':', linewidth=2, label = 'SCM')

# plt.title(r'Pressure Drop across Face B''\n' 'for the non-deformed test bundle', fontsize=13)
# plt.xlabel(r'$Axial~Segment~[wire~Pitch~from~heated~entry]$', fontsize=14)
# plt.ylabel(r'Pressure Drop [Pa]', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.ylim(0, 35)
# plt.xlim(-4, 9)
# plt.yticks(np.arange(0, 36, 5), fontsize=14)  # Set y-axis ticks every 1, starting from 0
# plt.xticks([-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9])
# # Set grid lines
# plt.grid(which='both', linestyle='--', linewidth=0.5)
# plt.savefig("DP-FaceB_ND.png")
# plt.show()

############################
############################
### Figure with temperatures at pin surfaces at plane B
plt.figure(figsize=(8, 5))
# Plot the data points without error bars
plt.plot(EXP_B[:, 0], EXP_B[:, 1], "ks", markerfacecolor='None', label="EXP")
plt.plot(EXP_B[:, 0], SC_TEMP - Mean, "rx", label="SC")

# Plot the error bars with dashed linestyle
plt.errorbar(EXP_B[:, 0], EXP_B[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')
plt.title(r'Temperature measurements at plane B''\n' 'on pin surface (4+6 o\'clock locations)', fontsize=13)
plt.xlabel(r'$Pin~Number~\#$', fontsize=14)
plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean [C]', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(-10, 20)

# Set major ticks at every 5 units
plt.xticks([1, 5, 10, 15, 20])

# Set minor ticks at every 1 unit
minor_ticks = range(int(min(EXP_B[:, 0])), int(max(EXP_B[:, 0])) + 1, 1)
plt.xticks(minor_ticks, minor=True)

# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)

# Save the figure with a specified DPI (e.g., 300)
plt.savefig("TEMP-PlaneB.png", dpi=300, bbox_inches='tight')  # Specify DPI directly in the savefig call
plt.show()
######################

#############################
#############################
#### Figure with temperatures at face E and plane B
plt.figure(figsize=(8, 5))
# Plot the data points without error bars
plt.plot(EXP_BE[:, 0], EXP_BE[:, 1], "ks", markerfacecolor= 'None', label="EXP")
plt.plot(EXP_BE[:, 0], SC_TEMP_BE - Mean, "rx", label="SC")
# Plot the error bars with dashed linestyle
plt.errorbar(EXP_BE[:, 0], EXP_BE[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')
plt.title(r"Temperature measurements at plane B and Face E", fontsize=13)
plt.xlabel(r'$TC~Number~\#$', fontsize=14)
plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean [C]', fontsize=14)
plt.legend(fontsize=12)
plt.xticks(range(1,6), fontsize=14)
plt.yticks(fontsize=14)
plt.ylim(-6, 4)
# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)
plt.savefig("TEMP-FaceE-PlaneB.png",dpi=300, bbox_inches='tight')
plt.show()
#################################

# #############################
# #############################
# ## Figure with temperatures at pin surfaces at plane C
# plt.figure(figsize=(8, 5))
# # Plot the data points without error bars
# plt.plot(EXP_C[:, 0], EXP_C[:, 1], "ks", markerfacecolor= 'None', label="EXP")
# plt.plot(EXP_C[:, 0], SC_TEMP - Mean, "rx", label="SCM")
# # Plot the error bars with dashed linestyle
# plt.errorbar(EXP_C[:, 0], EXP_C[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')
# plt.title(r'Temperature measurements at plane C''\n' 'on pin surface (8+10 o\'clock locations)', fontsize=13)
# plt.xlabel(r'$Pin~Number~\#$', fontsize=14)
# plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean [C]', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.ylim(-10, 20)
# # Set major ticks at every 5 units
# plt.xticks([1, 5, 10, 15, 20])
# # Set minor ticks at every 1 unit
# minor_ticks = range(int(min(EXP_C[:, 0])), int(max(EXP_C[:, 0])) + 1, 1)
# plt.xticks(minor_ticks, minor=True)
# # Set grid lines
# plt.grid(which='both', linestyle='--', linewidth=0.5)
# plt.savefig("TEMP-PlaneC.png", dpi=300, bbox_inches='tight')
# plt.show()
# ######################

# ##############################
# ##############################
# #### Figure with temperatures at face E and plane C
# plt.figure(figsize=(8, 5))
# # Plot the data points without error bars
# plt.plot(EXP_CE[:, 0], EXP_CE[:, 1], "ks", markerfacecolor= 'None', label="EXP")
# plt.plot(EXP_CE[:, 0], SC_TEMP_BE - Mean, "rx", label="SCM")
# # Plot the error bars with dashed linestyle
# plt.errorbar(EXP_CE[:, 0], EXP_CE[:, 1], yerr=error, fmt="none", ecolor="k", capsize=5, linestyle='dashdot')

# plt.title(r"Temperature measurements at plane C and Face E", fontsize=13)
# plt.xlabel(r'$TC~Number~\#$', fontsize=14)
# plt.ylabel(r'Temperature Difference from' '\n' 'Planar Mean [C]', fontsize=14)
# plt.legend(fontsize=12)
# plt.xticks(range(1,6), fontsize=14)
# plt.yticks(fontsize=14)
# plt.ylim(-6, 4)
# # Set grid lines
# plt.grid(which='both', linestyle='--', linewidth=0.5)
# plt.savefig("TEMP-FaceE-PlaneC.png", dpi=300, bbox_inches='tight')
# plt.show()
# ########################################
