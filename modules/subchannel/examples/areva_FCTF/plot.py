import numpy as np
import matplotlib.pyplot as plt

# Define the function
def sine_function(x):
    return (0.4*np.pi/(np.pi-2))*np.sin(x) + 1.4 - (0.4*np.pi/(np.pi-2))

# Generate x values
x_values = np.linspace(0, np.pi, 1000)

# Generate y values
y_values = sine_function(x_values)

# Plot the function
plt.plot(x_values, y_values, label='Axial power lot'
         )
plt.xlabel('x')
plt.ylabel('y')
plt.title('Sine Function')
plt.legend()
plt.show()
