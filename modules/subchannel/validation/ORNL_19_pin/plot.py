##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.ticker import MultipleLocator


############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################
SCM = np.genfromtxt("ORNL_19_out.csv", skip_header=2, delimiter=',')

Tin = 600 #F
DT = 153 #F

EXP  = np.array([0.8856, 0.81871, 0.98986, 1.1610, 1.261, 1.2851, 1.02203, 0.80572])
T_exp_F = EXP*DT + Tin
print(T_exp_F)
T_exp_K = (T_exp_F - 32)/1.8 + 273.15
subchannels = np.array([37, 36, 20, 10, 4, 1, 14, 28])

marker_size = 5  # You can adjust this value to change the marker size

# Plotting
plt.figure()
plt.plot(T_exp_K, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.xticks(ticks=[i for i in range(len(subchannels))], labels=[str(int(i)) for i in subchannels])
# plt.plot(snumber, explicit, "ro:", label="SCM calculation (explicit-segregated)", markersize=marker_size)  # "ro-" adds red round markers
# plt.plot(snumber, implicit, "go:", label="SCM calculation (implicit-segregated)", markersize=marker_size)  # "go-" adds green round markers
plt.plot(SCM[2:], "co:", label="SCM calculation (monolithic)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.plot(snumber, full_monolithic, "bo:", label="SCM calculation (full-monolithic)", markersize=marker_size)  # "bo-" adds blue round markers
plt.title(r"Temperature profile at the exit of the FFM-2A assembly" "\n" "Experiment date 022472", fontsize=13)
plt.xlabel(r'$Subchannel~\#$', fontsize=14)
plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
plt.legend(fontsize=8, loc='upper left')  # Legend placed at the top left
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
# plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
plt.gca().yaxis.set_major_locator(MultipleLocator(5))  # Set major ticks locator for Y-axis with increments of 5
plt.grid()
plt.tight_layout()
plt.savefig("temp.png")
plt.show()

# # Plotting
# plt.figure()
# plt.plot(snumber, EXP, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
# plt.plot(snumber, monolithic_10, "ro:", label="SCM calculation (monolithic 10 cells, β = 0.08)", markersize=marker_size)  # "ro-" adds red round markers
# plt.plot(snumber, monolithic_20, "go:", label="SCM calculation (monolithic 20 cells, β = 0.08)", markersize=marker_size)  # "go-" adds green round markers
# plt.plot(snumber, monolithic, "co:", label="SCM calculation (monolithic 40 cells, β = 0.08)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.plot(snumber, monolithic_beta, "co-", label="SCM calculation (monolithic 40 cells, β = 0.04)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.title(r"Temperature profile at the exit of the PSBT subassembly" "\n" "Experiment 01-5125, Run No 16", fontsize=13)
# plt.xlabel(r'$Subchannel~\#$', fontsize=14)
# plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
# plt.legend(fontsize=6)
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
# plt.grid()
# plt.savefig("temp2.png")
# plt.show()

# def q(p, r21, r32, s):
#     return np.log((r21**p - s) / (r32**p - s))

# def p_next(epsilon32, epsilon21, p_current, r21, r32):
#     s = np.sign(epsilon32 / epsilon21)
#     log_term = np.log(np.abs(epsilon32 / epsilon21))
#     q_val = q(p_current, r21, r32, s)
#     return np.abs(log_term + q_val)/ np.log(r21)

# def fixed_point_iteration(epsilon32, epsilon21, r21=2, r32=2, tol=1e-6, max_iter=1000):
#     p_current = 1.0  # Initial guess for p
#     for iteration in range(max_iter):
#         p_new = p_next(epsilon32, epsilon21, p_current, r21, r32)
#         if np.abs(p_new - p_current) < tol:
#             return p_new, iteration + 1
#         p_current = p_new
#     raise ValueError("Fixed-point iteration did not converge within the maximum number of iterations.")


# # Example usage:
# epsilon32 = 391
# epsilon21 = 184

# try:
#     p, num_iterations = fixed_point_iteration(epsilon32, epsilon21)
#     print(f"Converged to p = {p} in {num_iterations} iterations.")
# except ValueError as e:
#     print(e)

# phi1 = 133430.8
# phi2 = 133614.6
# phi3 = 134006.0
# r21 = 2
# r32 = 2

# rel_error = np.abs((phi1-phi2)/phi1)

# # Calculate \phi^{21}_{ext}
# phi_21_ext = (r21**p * phi1 - phi2) / (r21**p - 1)

# # Calculate \phi^{32}_{ext}
# phi_32_ext = (r32**p * phi2 - phi3) / (r32**p - 1)

# ext_rel_error = np.abs((phi_21_ext-phi1)/phi_21_ext)

# CGI = 1.25 * rel_error / (r21**p - 1)

# print(f"phi^21_ext = {phi_21_ext}")
# print(f"phi^32_ext = {phi_32_ext}")

# print(f"Aproximate Relative error = {rel_error}")
# print(f"Extrapolated Relative error = {ext_rel_error}")
# print(f"CGI = {CGI}")
