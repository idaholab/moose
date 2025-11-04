import numpy as np
import matplotlib.pyplot as plt

# Define the function
def sine_function(x):
    return (0.4*np.pi/(np.pi-2))*np.sin(np.pi*x/1.71) + 1.4 - (0.4*np.pi/(np.pi-2))

# Generate x values
x_values = np.linspace(0, 1.71, 100)

# Generate y values
y_values = sine_function(x_values)

# Plot the function
plt.plot(x_values, y_values, label='Axial power plot')
plt.xlabel('z [m]')
plt.ylabel('Normalized Heat Flux')
plt.title('Pin power profile')
plt.xticks(fontsize=14)
plt.yticks(fontsize=14)
# Set grid lines
plt.grid(which='both', linestyle='--', linewidth=0.5)
plt.show()
plt.savefig("power_profile.png")
plt.close()
