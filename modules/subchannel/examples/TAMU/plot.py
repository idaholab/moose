import numpy as np
import matplotlib.pyplot as plt

total_length = 2.0
rod_diameter = 1.59e-2
pitch = 1.89e-2
hwire = 0.476
p_over_d = pitch / rod_diameter
h_over_d = hwire / rod_diameter
ReL = pow(10, (p_over_d - 1.0)) * 320.0
ReT = pow(10, 0.7 * (p_over_d - 1)) * 1.0E+4
CFBL = (-974.6 + 1612.0 * p_over_d - 598.5 * pow(p_over_d, 2.0)) * pow(h_over_d, 0.06 - 0.085 * p_over_d)
CFBT = (0.8063 - 0.9022 * np.log(h_over_d) + 0.3526 * np.log(h_over_d) * np.log(h_over_d)) * pow(p_over_d, 9.7) * pow(h_over_d, 1.78 - 2.0 * p_over_d)

# Generate x values
Re_values = np.linspace(0, 20000, 1000)

# Define the function
def Friction(Re):
    fL = CFBL/Re
    fT =  CFBT / pow(Re, 0.18)
    if Re < ReL:
       return fL
    elif Re > ReT:
        return fT
    else:
        psi = np.log(Re / ReL) / np.log(ReT / ReL)
        return fL * pow((1 - psi), 1.0 / 3.0) * (1 - pow(psi, 7.0)) + fT * pow(psi, 1.0 / 3.0)

# Calculate y values
y_values = [Friction(Re) for Re in Re_values]

# Plot the function
plt.plot(Re_values, y_values, label='Friction factor plot')
plt.xlabel('Reynolds Number')
plt.ylabel('Friction Factor')
plt.title('Chen-Todreas Friction Factor Correlation (UCTS)')
plt.legend()
plt.grid(True)
plt.yscale('log')  # Set y-axis to logarithmic scale
plt.xscale('log')  # Set x-axis to logarithmic scale
plt.show()

# Print the last value of y_values
print("Last value of y_values:", y_values[-1])

print("p_over_d:", p_over_d)
print("h_over_d:", h_over_d)

