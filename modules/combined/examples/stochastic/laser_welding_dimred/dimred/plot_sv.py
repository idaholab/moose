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

sol_sv = data["time_steps"][0]["svd_output_sol"]["singular_values"]

colors = ['b','g','k']
markers = ['^','o','v']

sol_data = [sol_sv["T"]]
sum_sol_data = [sum([j*j for j in sol_data[i]]) for i in range(len(sol_data))]

sol_data = [i for i in sol_data]
error = [i for i in sol_data]
for i in range(len(sum_sol_data)):
  sol_data[i] = [j*j/sum_sol_data[i] for j in sol_data[i]]
  error[i] = [(1.0 - sum(sol_data[i][0:(k+1)])) for k in range(len(sol_data[i]))]

x = [i for i in range(1,12)]

# Create a figure and axis object
fig, ax = plt.subplots(figsize=(8, 6))
ax2 = ax.twinx()

for i in range(len(sol_data)):
  # Plotting the singular values
  ax.semilogy(x,sol_data[i][0:20], marker=markers[i], linestyle='-', color=colors[i],
          markersize=8, linewidth=2, label=r'$\mathrm{Normalized~Square~SV}$')

for i in range(len(sol_data)):
  # Plotting the singular values
  ax2.semilogy(x[0:(len(x)-1)],error[i][0:10], marker='>', linestyle='-.', color=colors[i+1],
          markersize=8, linewidth=2, label=r'$\mathrm{Projection~Error}$')

lines, labels = ax.get_legend_handles_labels()
lines2, labels2 = ax2.get_legend_handles_labels()

# Set custom axis ranges
ax.set_xlim(1, 11)
ax.set_ylim(1e-10, 1)
ax2.set_xlim(1, 11)
ax2.set_ylim(1e-10, 1)
ax.set_xticks(x)
ax.tick_params(axis='y', which='both')
ax.tick_params(axis='both', which='major', labelsize=20)
ax2.tick_params(axis='both', which='major', labelsize=20)

ax.set_xlabel(r'$\mathrm{Index}$', fontsize=20)
ax.set_ylabel(r'$\mathrm{Normalized~Squared~Singular~Value}$', fontsize=20)
ax2.set_ylabel(r'$\mathrm{Projection~Error}$', fontsize=20)


# Set grid
ax.grid(True, linestyle='--', alpha=0.5)

# Add legend
ax.legend(lines + lines2, labels + labels2, fontsize=20, loc='best')

# Tight layout
fig.tight_layout()

# Save plot to PDF
plt.savefig('sol_aux.pdf')
