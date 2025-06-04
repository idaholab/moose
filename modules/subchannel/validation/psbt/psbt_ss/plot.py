##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator, MultipleLocator


############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################


EXP = np.array([603.75, 604.35, 602.35, 598.05, 595.55, 592.35, 604.65, 605.15, 602.75, 598.95, 596.65, 594.55, 602.95, 602.65, 602.55, 598.95, 594.15, 593.65, 602.25, 601.35, 599.35, 594.35, 593.05, 591.75, 602.35, 601.65, 599.05, 595.65, 592.75, 590.55, 602.65, 601.95, 595.25, 594.15, 590.75, 589.25])

explicit = np.array([602.1973757, 601.3602789, 598.8783678, 595.6451569, 593.1371039, 592.0609153,
                            602.6313027, 601.9223523, 599.4254542, 595.9699282, 593.2031263, 592.0167427,
                            602.8973407, 602.2216452, 599.7256472, 596.1787146, 593.2849875, 592.0411726,
                            602.8973407, 602.2216452, 599.7256472, 596.1787146, 593.2849875, 592.0411726,
                            602.6313027, 601.9223523, 599.4254542, 595.9699282, 593.2031263, 592.0167427,
                            602.1973757, 601.3602789, 598.8783678, 595.6451569, 593.1371039, 592.0609153])

implicit = np.array([602.1488735, 601.3466393, 598.8597127, 595.6120922, 593.0990688, 592.0065868,
                  602.6166397, 601.9513247, 599.4643749, 595.9808744, 593.177899, 591.9716526,
                  602.9039575, 602.2768425, 599.7874118, 596.2062233, 593.2714308, 592.0030376,
                  602.9039575, 602.2768425, 599.7874118, 596.2062233, 593.2714308, 592.0030376,
                  602.6166397, 601.9513247, 599.4643749, 595.9808744, 593.177899, 591.9716526,
                  602.1488735, 601.3466393, 598.8597127, 595.6120922, 593.0990688, 592.0065868])

monolithic_100 = np.array([602.3359876, 601.4766814, 598.9273979, 595.6443327, 593.1199195, 592.035565,
                  602.7821506, 602.0420667, 599.4781694, 595.9681302, 593.1763195, 591.9874143,
                  603.0513458, 602.3434403, 599.7755996, 596.1706904, 593.2523361, 592.0065035,
                  603.051346, 602.3434402, 599.7755996, 596.1706901, 593.2523344, 592.0065031,
                  602.7821493, 602.0420663, 599.4781684, 595.9681302, 593.1763203, 591.9874144,
                  602.3359871, 601.4766802, 598.927397, 595.6443322, 593.1199184, 592.0355642])

full_monolithic = data = np.array([602.3361566, 601.4769487, 598.9274476, 595.6443185, 593.1204734, 592.0360876,
                  602.7819213, 602.0419916, 599.478248, 595.9683724, 593.1770496, 591.9880454,
                  603.0503954, 602.3428363, 599.7755032, 596.170872, 593.2529656, 592.0069479,
                  603.0503919, 602.342856, 599.7754956, 596.1708762, 593.2529341, 592.0069406,
                  602.7818744, 602.0420275, 599.4782569, 595.9683696, 593.1770129, 591.9881133,
                  602.3361189, 601.4770073, 598.9274493, 595.6443131, 593.1204112, 592.0361178])

monolithic_20 = np.array([601.6536139, 600.8127089, 598.8354655, 596.1643723, 593.6741581, 592.3841614,
                 602.0528889, 601.2638217, 599.1644122, 596.4273162, 593.9268542, 592.4942826,
                 602.3381394, 601.5726738, 599.4448958, 596.6399673, 594.0625829, 592.5726406,
                 602.3381364, 601.572677, 599.4449017, 596.6399683, 594.0625775, 592.57264,
                 602.0528873, 601.2638186, 599.164407, 596.4273121, 593.9268515, 592.4942777,
                 601.6536119, 600.8127062, 598.8354622, 596.1643724, 593.6741547, 592.3841574])

monolithic_40 = np.array([602.3566491, 601.3788544, 598.9881827, 595.9153446, 593.4271503, 592.3668362,
                 602.6848717, 601.7473961, 599.2704802, 596.0315822, 593.4040832, 592.275228,
                 602.9204546, 602.0049206, 599.5115169, 596.1797285, 593.438718, 592.2611283,
                 602.9204513, 602.0049179, 599.5115143, 596.1797264, 593.4387156, 592.2611277,
                 602.6848693, 601.7473929, 599.2704754, 596.0315791, 593.404078, 592.2752218,
                 602.3566433, 601.3788515, 598.988177, 595.9153426, 593.4271433, 592.366832])

monolithic_100_beta = np.array([605.647218, 604.3217005, 599.8212529, 593.8916767, 589.4657638, 587.5756606,
                 606.4885382, 605.422234, 600.9979101, 594.6440757, 589.6337094, 587.5440737,
                 606.9564293, 605.9549483, 601.5550758, 595.0466281, 589.8014304, 587.607467,
                 606.9564358, 605.9549582, 601.5550886, 595.0466253, 589.8014276, 587.607464,
                 606.4885402, 605.4222487, 600.9979181, 594.6440638, 589.6336924, 587.5440748,
                 605.6472153, 604.3217049, 599.8212645, 593.8916804, 589.4657733, 587.575675])

snumber = np.array([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36])


marker_size = 3  # You can adjust this value to change the marker size

# Plotting the first figure
plt.figure()
plt.plot(snumber, EXP, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.plot(snumber, explicit, "ro:", label="SCM calculation (explicit-segregated)", markersize=marker_size)  # "ro-" adds red round markers
plt.plot(snumber, implicit, "go:", label="SCM calculation (implicit-segregated)", markersize=marker_size)  # "go-" adds green round markers
plt.plot(snumber, monolithic_100, "co:", label="SCM calculation (monolithic)", markersize=marker_size)  # "co-" adds cyan round markers
plt.plot(snumber, full_monolithic, "bo:", label="SCM calculation (full-monolithic)", markersize=marker_size)  # "bo-" adds blue round markers
plt.title(r"Temperature profile at the exit of the PSBT subassembly" "\n" "Experiment 01-5125, Run No 16", fontsize=13)
plt.xlabel(r'$Subchannel~\#$', fontsize=14)
plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
plt.legend(fontsize=6)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
plt.gca().yaxis.set_major_locator(MultipleLocator(2))  # Set grid size to 1
plt.grid()
plt.savefig("temp.png")
plt.show()

# Plotting the second figure
plt.figure()
plt.plot(snumber, EXP, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.plot(snumber, monolithic_20, "ro:", label="SCM calculation (monolithic 20 cells, β = 0.08)", markersize=marker_size)  # "ro-" adds red round markers
plt.plot(snumber, monolithic_40, "go:", label="SCM calculation (monolithic 40 cells, β = 0.08)", markersize=marker_size)  # "go-" adds green round markers
plt.plot(snumber, monolithic_100, "co:", label="SCM calculation (monolithic 100 cells, β = 0.08)", markersize=marker_size)  # "co-" adds cyan round markers
plt.plot(snumber, monolithic_100_beta, "co-", label="SCM calculation (monolithic 100 cells, β = 0.04)", markersize=marker_size)  # "co-" adds cyan round markers
plt.title(r"Temperature profile at the exit of the PSBT subassembly" "\n" "Experiment 01-5125, Run No 16", fontsize=13)
plt.xlabel(r'$Subchannel~\#$', fontsize=14)
plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
plt.legend(fontsize=6)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
plt.gca().yaxis.set_major_locator(MultipleLocator(2))  # Set grid size to 1
plt.grid()
plt.savefig("temp2.png")
plt.show()

def q(p, r21, r32, s):
    term = (r21**p - s) / (r32**p - s)
    if term <= 0:
        raise ValueError("The term for the logarithm must be positive and non-zero.")
    return np.log(term)

def p_next(epsilon32, epsilon21, p_current, r21, r32):
    s = np.sign(epsilon32 / epsilon21)
    log_term = np.log(np.abs(epsilon32 / epsilon21))
    q_val = q(p_current, r21, r32, s)
    return np.abs(log_term + q_val) / np.log(r21)

def fixed_point_iteration(epsilon32, epsilon21, r21=2, r32=2, tol=1e-6, max_iter=1000):
    p_current = 1.0  # Initial guess for p
    for iteration in range(max_iter):
        p_new = p_next(epsilon32, epsilon21, p_current, r21, r32)
        print(f"Iteration {iteration}: p_current = {p_current}, p_new = {p_new}")  # Debugging output

        if np.abs(p_new - p_current) < tol:
            return p_new, iteration + 1
        p_current = p_new
    raise ValueError("Fixed-point iteration did not converge within the maximum number of iterations.")

phi1 = 133302.1356
phi2 = 133430.7598
phi3 = 133614.6141

h1 = 0.013534472
h2 = 0.018369104
h3 = 0.023143621

# r21 = h2/h1
# r32 = h3/h1

r21 = 100/40
r32 = 40/20

print(r21)
print(r32)

# Example usage:
epsilon32 = phi3 - phi2
epsilon21 = phi2 - phi1

try:
    p, num_iterations = fixed_point_iteration(epsilon32, epsilon21, r21, r32)
    print(f"Converged to p = {p} in {num_iterations} iterations.")
except ValueError as e:
    print(e)

rel_error = np.abs((phi1-phi2) / phi1)

# Calculate \phi^{21}_{ext}
phi_21_ext = (r21**p * phi1 - phi2) / (r21**p - 1)

# Calculate \phi^{32}_{ext}
phi_32_ext = (r32**p * phi2 - phi3) / (r32**p - 1)

ext_rel_error = np.abs((phi_21_ext - phi1) / phi_21_ext)

CGI = 1.25 * rel_error / (r21**p - 1)

print(f"phi^21_ext = {phi_21_ext}")
print(f"phi^32_ext = {phi_32_ext}")

print(f"Approximate Relative error = {rel_error}")
print(f"Extrapolated Relative error = {ext_rel_error}")
print(f"CGI = {CGI}")
