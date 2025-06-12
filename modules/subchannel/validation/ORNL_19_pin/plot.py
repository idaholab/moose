##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.ticker import MultipleLocator


############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
# ###############################################
# monolithic_20 = np.genfromtxt("monolithic_20.csv", skip_header=2, delimiter=',')
# monolithic_40 = np.genfromtxt("monolithic_40.csv", skip_header=2, delimiter=',')
# monolithic_100 = np.genfromtxt("monolithic_100.csv", skip_header=2, delimiter=',')
# monolithic_140 = np.genfromtxt("monolithic_140.csv", skip_header=2, delimiter=',')

# full_monolithic_20 = np.genfromtxt("full_monolithic_20.csv", skip_header=2, delimiter=',')
# full_monolithic_40 = np.genfromtxt("full_monolithic_40.csv", skip_header=2, delimiter=',')
# full_monolithic_100 = np.genfromtxt("full_monolithic_100.csv", skip_header=2, delimiter=',')

# explicit_20 = np.genfromtxt("explicit_20.csv", skip_header=2, delimiter=',')
# explicit_40 = np.genfromtxt("explicit_40.csv", skip_header=2, delimiter=',')
# explicit_100 = np.genfromtxt("explicit_100.csv", skip_header=2, delimiter=',')

# implicit_20 = np.genfromtxt("implicit_20.csv", skip_header=2, delimiter=',')
# implicit_40 = np.genfromtxt("implicit_40.csv", skip_header=2, delimiter=',')
# implicit_100 = np.genfromtxt("implicit_100.csv", skip_header=2, delimiter=',')

current = np.genfromtxt("ORNL_19_out.csv", skip_header=2, delimiter=',')

Tin = 600 #F
DT = 153 #F

EXP  = np.array([0.8856, 0.81871, 0.98986, 1.1610, 1.261, 1.2851, 1.02203, 0.80572])
T_exp_F = EXP*DT + Tin
print(T_exp_F)
T_exp_K = (T_exp_F - 32)/1.8 + 273.15
subchannels = np.array([37, 36, 20, 10, 4, 1, 14, 28])

marker_size = 5  # You can adjust this value to change the marker size

## Plotting
plt.figure()
plt.plot(T_exp_K, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.xticks(ticks=[i for i in range(len(subchannels))], labels=[str(int(i)) for i in subchannels])
# plt.plot(explicit_100[3:], "ro:", label="SCM calculation (explicit-segregated)", markersize=marker_size)  # "ro-" adds red round markers
# plt.plot(implicit_100[3:], "go-", label="SCM calculation (implicit-segregated)", markersize=marker_size)  # "go-" adds green round markers
# plt.plot(monolithic_100[3:], "co:", label="SCM calculation (monolithic)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.plot(full_monolithic_100[3:], "bo:", label="SCM calculation (full-monolithic)", markersize=marker_size)  # "bo-" adds blue round markers
plt.plot(current[3:11], "co:", label="SCM calculation (monolithic)", markersize=marker_size)  # "co-" adds cyan round markers
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
plt.savefig("temp3.png")
plt.show()

# Plotting
# plt.figure()
# plt.plot(T_exp_K, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
# plt.xticks(ticks=[i for i in range(len(subchannels))], labels=[str(int(i)) for i in subchannels])
# plt.plot(monolithic_20[3:], "ro:", label="SCM calculation (monolithic 20 cells)", markersize=marker_size)  # "ro-" adds red round markers
# plt.plot(monolithic_40[3:], "go:", label="SCM calculation (monolithic 40 cells)", markersize=marker_size)  # "go-" adds green round markers
# plt.plot(monolithic_100[3:], "co:", label="SCM calculation (monolithic 100 cells)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.plot(monolithic_140[3:], "ko:", label="SCM calculation (monolithic 140 cells)", markersize=marker_size)  # "co-" adds cyan round markers
# plt.title(r"Temperature profile at the exit of the FFM-2A assembly" "\n" "Experiment date 022472", fontsize=13)
# plt.xlabel(r'$Subchannel~\#$', fontsize=14)
# plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
# plt.legend(fontsize=8, loc='upper left')  # Legend placed at the top left
# plt.xticks(fontsize=14)
# plt.yticks(fontsize=14)
# # plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
# plt.gca().yaxis.set_major_locator(MultipleLocator(5))  # Set major ticks locator for Y-axis with increments of 5
# plt.grid()
# plt.tight_layout()
# plt.savefig("temp4.png")
# plt.show()

# def q(p, r21, r32, s):
#     term = (r21**p - s) / (r32**p - s)
#     if term <= 0:
#         raise ValueError("The term for the logarithm must be positive and non-zero.")
#     return np.log(term)

# def p_next(epsilon32, epsilon21, p_current, r21, r32):
#     s = np.sign(epsilon32 / epsilon21)
#     log_term = np.log(np.abs(epsilon32 / epsilon21))
#     q_val = q(p_current, r21, r32, s)
#     return np.abs(log_term + q_val) / np.log(r21)

# def fixed_point_iteration(epsilon32, epsilon21, r21=2, r32=2, tol=1e-6, max_iter=1000):
#     p_current = 1.0  # Initial guess for p
#     for iteration in range(max_iter):
#         p_new = p_next(epsilon32, epsilon21, p_current, r21, r32)
#         print(f"Iteration {iteration}: p_current = {p_current}, p_new = {p_new}")  # Debugging output

#         if np.abs(p_new - p_current) < tol:
#             return p_new, iteration + 1
#         p_current = p_new
#     raise ValueError("Fixed-point iteration did not converge within the maximum number of iterations.")

# h1 = 0.004837278
# h2 = 0.006565196
# h3 = 0.008271628

# phi1 = 149341.37974882
# phi2 = 149311.19945983
# phi3 = 149318.58

# r21 = 100/40
# r32 = 40/20

# print(r21)
# print(r32)

# # Example usage:
# epsilon32 = phi3 - phi2
# epsilon21 = phi2 - phi1

# try:
#     p, num_iterations = fixed_point_iteration(epsilon32, epsilon21, r21, r32)
#     print(f"Converged to p = {p} in {num_iterations} iterations.")
# except ValueError as e:
#     print(e)

# rel_error = np.abs((phi1-phi2) / phi1)

# # Calculate \phi^{21}_{ext}
# phi_21_ext = (r21**p * phi1 - phi2) / (r21**p - 1)

# # Calculate \phi^{32}_{ext}
# phi_32_ext = (r32**p * phi2 - phi3) / (r32**p - 1)

# ext_rel_error = np.abs((phi_21_ext - phi1) / phi_21_ext)

# CGI = 1.25 * rel_error / (r21**p - 1)

# print(f"phi^21_ext = {phi_21_ext}")
# print(f"phi^32_ext = {phi_32_ext}")

# print(f"Approximate Relative error = {rel_error}")
# print(f"Extrapolated Relative error = {ext_rel_error}")
# print(f"CGI = {CGI}")
