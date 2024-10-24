import numpy as np
import matplotlib.pyplot as plt
import json
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "sans-serif",
    "font.sans-serif": "Helvetica",
})

data = {}
with open("pod_mapping_train_out.json", 'r') as file:
  # Load the JSON data
  data = json.load(file)

aux_sv = data["time_steps"][0]["svd_output_aux"]["singular_values"]
sol_sv = data["time_steps"][0]["svd_output_sol"]["singular_values"]

colors = ['b','g','k']
markers = ['^','o','v']
aux_names = [r'$\mathrm{Velocity~x}$', r'$\mathrm{Velocity~y}$']
aux_data = [aux_sv["vel_x_aux"],aux_sv["vel_y_aux"]]
sum_aux_data = [sum([j*j for j in aux_data[i]]) for i in range(len(aux_data))]
for i in range(len(sum_aux_data)):
  aux_data[i] = [j*j/sum_aux_data[i] for j in aux_data[i]]

sol_names = [r'$\mathrm{Temperature}$', r'$\mathrm{Displacement~x}$', r'$\mathrm{Displacement~y}$']
sol_data = [sol_sv["T"],sol_sv["disp_x"],sol_sv["disp_y"]]
sum_sol_data = [sum([j*j for j in sol_data[i]]) for i in range(len(sol_data))]
sol_data = [i for i in sol_data]
for i in range(len(sum_sol_data)):
  sol_data[i] = [j*j/sum_sol_data[i] for j in sol_data[i]]

# Create a figure and axis object
fig, ax = plt.subplots(figsize=(8, 6))
x = [i for i in range(1,21)]

for i in range(len(aux_data)):
  # Plotting the singular values
  ax.semilogy(x,aux_data[i][0:20], marker=markers[i], linestyle='-', color=colors[i],
          markersize=8, linewidth=2, label=aux_names[i])

# Set custom axis ranges
ax.set_xlim(1, 20)
ax.set_ylim(1e-8, 1)
ax.set_xticks(x)
ax.tick_params(axis='y', which='both')
ax.tick_params(axis='both', which='major', labelsize=20)

ax.set_xlabel(r'$\mathrm{Index}$', fontsize=20)
ax.set_ylabel(r'$\mathrm{Normalized~Squared~Singular~Value}$', fontsize=20)


# Set grid
ax.grid(True, linestyle='--', alpha=0.5)

# Add legend
ax.legend(fontsize=20, loc='best')

# Tight layout
fig.tight_layout()

# Save plot to PDF
plt.savefig('vel_aux.pdf')

# Create a figure and axis object
fig, ax = plt.subplots(figsize=(8, 6))

for i in range(len(sol_data)):
  # Plotting the singular values
  ax.semilogy(x,sol_data[i][0:20], marker=markers[i], linestyle='-', color=colors[i],
          markersize=8, linewidth=2, label=sol_names[i])

# Set custom axis ranges
ax.set_xlim(1, 20)
ax.set_ylim(1e-10, 1)
ax.set_xticks(x)
ax.tick_params(axis='y', which='both')
ax.tick_params(axis='both', which='major', labelsize=20)

ax.set_xlabel(r'$\mathrm{Index}$', fontsize=20)
ax.set_ylabel(r'$\mathrm{Normalized~Squared~Singular~Value}$', fontsize=20)


# Set grid
ax.grid(True, linestyle='--', alpha=0.5)

# Add legend
ax.legend(fontsize=20, loc='best')

# Tight layout
fig.tight_layout()

# Save plot to PDF
plt.savefig('sol_aux.pdf')
