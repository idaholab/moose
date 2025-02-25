import numpy as np
import matplotlib.pyplot as plt

fname = 'marvel_loop_mod_out.csv'
sam = np.genfromtxt(fname, delimiter=',', dtype=None, names=True)

fig, ax1 = plt.subplots()

ax1.plot(sam["time"]/60, sam["pressure_drop_SAM"], color='b', label=r"SAM")
ax1.plot(sam["time"]/60, sam["core_delta_p_tgt"], color='r', linestyle='--', label="SCM PP")
ax1.set_xlabel("Time [min]", fontsize=12)
ax1.set_ylabel("Pressure Drop [Pa]", fontsize=12, color='b')
ax1.tick_params(axis='y', labelcolor='b')
ax1.grid()

# Create a second y-axis
ax2 = ax1.twinx()
ax2.plot(sam["time"]/60, sam["inlet_mass_flux_final"], color='g', label="SCM mass-flux")
ax2.set_ylabel("Inlet Mass Flux [kg/m^2/s]", fontsize=12, color='g')
ax2.tick_params(axis='y', labelcolor='g')

# Combine both legends and place them at the bottom right
lines, labels = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines + lines2, labels + labels2, loc='lower right', fontsize=10)

fig.tight_layout()
plt.savefig('./pressure_drop_monitor.png', bbox_inches='tight')
plt.show()

fname2 = 'marvel_loop_mod2_out.csv'
sam2 = np.genfromtxt(fname2, delimiter=',', dtype=None, names=True)

fig, ax1 = plt.subplots()

ax1.plot(sam2["time"]/60, sam2["pressure_drop_SAM"], color='b', label=r"SAM")
ax1.plot(sam2["time"]/60, sam2["core_delta_p_tgt"], color='r', linestyle='--', label="SCM PP")
ax1.set_xlabel("Time [min]", fontsize=12)
ax1.set_ylabel("Pressure Drop [Pa]", fontsize=12, color='b')
ax1.tick_params(axis='y', labelcolor='b')
ax1.grid()

# Create a second y-axis
ax2 = ax1.twinx()
ax2.plot(sam2["time"]/60, sam2["inlet_mass_flux_final"], color='g', label="SCM mass-flux")
ax2.set_ylabel("Inlet Mass Flux [kg/m^2/s]", fontsize=12, color='g')
ax2.tick_params(axis='y', labelcolor='g')

# Combine both legends and place them at the bottom right
lines, labels = ax1.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()
ax1.legend(lines + lines2, labels + labels2, loc='lower right', fontsize=10)

fig.tight_layout()
plt.savefig('./pressure_drop_monitor2.png', bbox_inches='tight')
plt.show()
