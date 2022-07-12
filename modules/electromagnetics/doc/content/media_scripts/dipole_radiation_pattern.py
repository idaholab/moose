import numpy as np
import matplotlib.pyplot as plt


theta = np.arange(0, 2*np.pi, 0.01*np.pi)
# Radiation intensity (directional component) for electrically small wire dipole antenna from Pozar (p. 644)
# This is approximately the same directionally as that in the half-wave case in Silver (p. 98-99)
I = np.sin(theta)**2

fig, ax = plt.subplots(subplot_kw={'projection': 'polar'})
ax.plot(theta, I)
ax.set_rmax(1)
ax.set_rticks([0.5, 1])  # Less radial ticks
ax.set_rlabel_position(-22.5)  # Move radial labels away from plotted line
ax.grid(True)
ax.set_theta_zero_location("N")

#plt.show()
fig.savefig("dipole_radiation_pattern.png")
