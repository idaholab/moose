##### LOAD MODULES ###############
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator


############### MAKE PRETTY ###################
plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "dejavuserif"
###############################################


EXP = np.array([603.75, 604.35, 602.35, 598.05, 595.55, 592.35, 604.65, 605.15, 602.75, 598.95, 596.65, 594.55, 602.95, 602.65, 602.55, 598.95, 594.15, 593.65, 602.25, 601.35, 599.35, 594.35, 593.05, 591.75, 602.35, 601.65, 599.05, 595.65, 592.75, 590.55, 602.65, 601.95, 595.25, 594.15, 590.75, 589.25])

explicit = np.array([602.1982039, 601.2161786, 598.9493221, 595.9214573, 593.3740232, 592.3111959, 602.5131522, 601.6252171, 599.2369856, 596.0440811, 593.3925304, 592.2339907, 602.7911465, 601.9356479, 599.5502117, 596.2579854, 593.478475, 592.2579938, 602.7911465, 601.9356479, 599.5502117, 596.2579854, 593.478475, 592.2579938, 602.5131522, 601.6252171, 599.2369856, 596.0440811, 593.3925304, 592.2339907, 602.1982039, 601.2161786, 598.9493221, 595.9214573, 593.3740232, 592.3111959])

implicit = np.array([602.0920171, 601.2242204, 598.881834, 595.8134531, 593.3327633, 592.257953, 602.4827833, 601.7089336, 599.3214515, 596.0556442, 593.3692479, 592.1986214, 602.7591558, 602.0208414, 599.6246751, 596.2647095, 593.4565849, 592.2256561, 602.7591558, 602.0208414, 599.6246751, 596.2647095, 593.4565849, 592.2256561, 602.4827833, 601.7089336, 599.3214515, 596.0556442, 593.3692479, 592.1986214, 602.0920171, 601.2242204, 598.881834, 595.8134531, 593.3327633, 592.257953])

monolithic = np.array([602.3566491, 601.3788544, 598.9881827, 595.9153446, 593.4271503, 592.3668362, 602.6848717, 601.7473961, 599.2704802, 596.0315822, 593.4040832, 592.275228, 602.9204546, 602.0049206, 599.5115169, 596.1797285, 593.438718, 592.2611283, 602.9204513, 602.0049179, 599.5115143, 596.1797264, 593.4387156, 592.2611277, 602.6848693, 601.7473929, 599.2704754, 596.0315791, 593.404078, 592.2752218, 602.3566433, 601.3788515, 598.988177, 595.9153426, 593.4271433, 592.366832])

full_monolithic = np.array([602.3603114, 601.3802745, 598.9905096, 595.9173501, 593.4279186, 592.3682672, 602.6867142, 601.7450095, 599.2690704, 596.03116, 593.4033413, 592.2758625, 602.9228428, 602.0028243, 599.5105245, 596.1797595, 593.4382486, 592.2618558, 602.9228454, 602.0028302, 599.5105378, 596.1797447, 593.4382375, 592.2618451, 602.6867224, 601.7450072, 599.2690605, 596.0311749, 593.4033435, 592.2758642, 602.3603047, 601.3802814, 598.9905003, 595.9173591, 593.4279107, 592.3682715])

monolithic_10 = np.array([600.6559739, 599.9590046, 598.6764574, 596.681456, 594.3922402, 593.1525832, 600.9569299, 600.2496781, 598.865665, 596.8898859, 594.6552341, 593.2993866, 601.3232305, 600.6605419, 599.2659182, 597.2601326, 594.9758138, 593.5408247, 601.3232436, 600.6605236, 599.2659242, 597.2601096, 594.9758244, 593.5408143, 600.9569142, 600.2496962, 598.8656375, 596.8899083, 594.6552184, 593.299395, 600.65599, 599.9589863, 598.6764762, 596.681438, 594.3922507, 593.152566])

monolithic_20 = np.array([601.6536139, 600.8127089, 598.8354655, 596.1643723, 593.6741581, 592.3841614, 602.0528889, 601.2638217, 599.1644122, 596.4273162, 593.9268542, 592.4942826, 602.3381394, 601.5726738, 599.4448958, 596.6399673, 594.0625829, 592.5726406, 602.3381364, 601.572677, 599.4449017, 596.6399683, 594.0625775, 592.57264, 602.0528873, 601.2638186, 599.164407, 596.4273121, 593.9268515, 592.4942777, 601.6536119, 600.8127062, 598.8354622, 596.1643724, 593.6741547, 592.3841574])

monolithic_beta = np.array([605.8883899,	604.33472,	599.9769918,	594.2409151,	589.8149008,	588.0024585,	606.5397083,	605.1163207,	600.7225167,	594.6319491,	589.7960544,	587.8609077,	606.9710557,	605.599677,	601.2172501,	594.9640244,	589.8956677,	587.8681456,	606.9710542,	605.5996776,	601.21725,	594.9640267,	589.8956629, 587.8681428,	606.5397032,	605.1163182,	600.7225123,	594.6319446,	589.7960528,	587.8609087,	605.8883794,	604.3347099,	599.9769853,	594.2409137,	589.8149,	588.0024576])

snumber = np.array([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36])


marker_size = 3  # You can adjust this value to change the marker size

# Plotting
plt.figure()
plt.plot(snumber, EXP, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.plot(snumber, explicit, "ro:", label="SCM calculation (explicit-segregated)", markersize=marker_size)  # "ro-" adds red round markers
plt.plot(snumber, implicit, "go:", label="SCM calculation (implicit-segregated)", markersize=marker_size)  # "go-" adds green round markers
plt.plot(snumber, monolithic, "co:", label="SCM calculation (monolithic)", markersize=marker_size)  # "co-" adds cyan round markers
plt.plot(snumber, full_monolithic, "bo:", label="SCM calculation (full-monolithic)", markersize=marker_size)  # "bo-" adds blue round markers
plt.title(r"Temperature profile at the exit of the PSBT subassembly" "\n" "Experiment 01-5125, Run No 16", fontsize=13)
plt.xlabel(r'$Subchannel~\#$', fontsize=14)
plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
plt.legend(fontsize=6)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
plt.grid()
plt.savefig("temp.png")
plt.show()

# Plotting
plt.figure()
plt.plot(snumber, EXP, "ks-", label="Experimental measurements", markersize=marker_size)  # "ks-" adds black square markers
plt.plot(snumber, monolithic_10, "ro:", label="SCM calculation (monolithic 10 cells, β = 0.08)", markersize=marker_size)  # "ro-" adds red round markers
plt.plot(snumber, monolithic_20, "go:", label="SCM calculation (monolithic 20 cells, β = 0.08)", markersize=marker_size)  # "go-" adds green round markers
plt.plot(snumber, monolithic, "co:", label="SCM calculation (monolithic 40 cells, β = 0.08)", markersize=marker_size)  # "co-" adds cyan round markers
plt.plot(snumber, monolithic_beta, "co-", label="SCM calculation (monolithic 40 cells, β = 0.04)", markersize=marker_size)  # "co-" adds cyan round markers
plt.title(r"Temperature profile at the exit of the PSBT subassembly" "\n" "Experiment 01-5125, Run No 16", fontsize=13)
plt.xlabel(r'$Subchannel~\#$', fontsize=14)
plt.ylabel(r'$T_{outlet}[K]$', fontsize=14)
plt.legend(fontsize=6)
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
plt.gca().yaxis.set_major_locator(MaxNLocator(integer=True))  # Force Y-axis to use integer locator
plt.grid()
plt.savefig("temp2.png")
plt.show()

def q(p, r21, r32, s):
    return np.log((r21**p - s) / (r32**p - s))

def p_next(epsilon32, epsilon21, p_current, r21, r32):
    s = np.sign(epsilon32 / epsilon21)
    log_term = np.log(np.abs(epsilon32 / epsilon21))
    q_val = q(p_current, r21, r32, s)
    return np.abs(log_term + q_val)/ np.log(r21)

def fixed_point_iteration(epsilon32, epsilon21, r21=2, r32=2, tol=1e-6, max_iter=1000):
    p_current = 1.0  # Initial guess for p
    for iteration in range(max_iter):
        p_new = p_next(epsilon32, epsilon21, p_current, r21, r32)
        if np.abs(p_new - p_current) < tol:
            return p_new, iteration + 1
        p_current = p_new
    raise ValueError("Fixed-point iteration did not converge within the maximum number of iterations.")


# Example usage:
epsilon32 = 391
epsilon21 = 184

try:
    p, num_iterations = fixed_point_iteration(epsilon32, epsilon21)
    print(f"Converged to p = {p} in {num_iterations} iterations.")
except ValueError as e:
    print(e)

phi1 = 133430.8
phi2 = 133614.6
phi3 = 134006.0
r21 = 2
r32 = 2

rel_error = np.abs((phi1-phi2)/phi1)

# Calculate \phi^{21}_{ext}
phi_21_ext = (r21**p * phi1 - phi2) / (r21**p - 1)

# Calculate \phi^{32}_{ext}
phi_32_ext = (r32**p * phi2 - phi3) / (r32**p - 1)

ext_rel_error = np.abs((phi_21_ext-phi1)/phi_21_ext)

CGI = 1.25 * rel_error / (r21**p - 1)

print(f"phi^21_ext = {phi_21_ext}")
print(f"phi^32_ext = {phi_32_ext}")

print(f"Aproximate Relative error = {rel_error}")
print(f"Extrapolated Relative error = {ext_rel_error}")
print(f"CGI = {CGI}")
