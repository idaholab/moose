import json
import matplotlib.pyplot as plt
import numpy as np

plt.rc('text', usetex=True)
plt.rc('font',**{'family':'sans-serif','sans-serif':['Helvetica']})

# Load data from JSON file
with open('train_out.json') as f:
    data = json.load(f)

# Extract data from JSON and select every other data point starting from the first
time_steps = data['time_steps'][1::2]  # Exclude the first entry and then take every other entry
average_rewards = [step['reward']['average_reward'] for step in time_steps]
std_rewards = [step['reward']['std_reward'] for step in time_steps]
sample_average_rewards = [step['reward']['sample_average_reward'] for step in time_steps]

# Create a plot
fig, ax = plt.subplots()

# Set LaTeX font


# Plot average reward data
indices = range(1, len(time_steps) + 1)  # Start numbering from 1
ax.plot(indices, average_rewards, label=r'$\mathrm{Average~Reward}$', color='darkblue', linewidth=2)

# Compute confidence intervals
lower_bound_1std = [avg - std for avg, std in zip(average_rewards, std_rewards)]
upper_bound_1std = [avg + std for avg, std in zip(average_rewards, std_rewards)]
lower_bound_2std = [avg - 2 * std for avg, std in zip(average_rewards, std_rewards)]
upper_bound_2std = [avg + 2 * std for avg, std in zip(average_rewards, std_rewards)]

# Fill between for confidence intervals
# ax.fill_between(indices, lower_bound_2std, upper_bound_2std, color='lightblue', alpha=0.75, label=r'$\pm 2\sigma$')
ax.fill_between(indices, lower_bound_1std, upper_bound_1std, color='lightblue', alpha=1.0, label=r'$\pm \sigma$')

# Plot sample average reward points
for i, sample_rewards in enumerate(sample_average_rewards, start=1):
    ax.scatter([i] * len(sample_rewards), sample_rewards, color='black', s=5, alpha=0.7, label=r'$\mathrm{Average~sample~rewards}$' if i == 1 else "")

# Set custom axis ranges (adjust as needed)
ax.set_xlim([1, len(time_steps)])  # Example range for x-axis
ax.set_ylim([min(lower_bound_1std) - 1, max(upper_bound_1std) + 1])  # Example range for y-axis

# Ensure x-axis uses only integers and includes the first and last indices
ax.set_xticks(np.arange(1, len(time_steps) + 1, step=1))

# Set custom axis titles
ax.set_xlabel(r'$\mathrm{Update~(10~Episodes)}$', fontsize=14)
ax.set_ylabel(r'$\mathrm{Average~Reward}$', fontsize=14)

# Set custom legend
ax.legend(loc='best')

# Save plot as PDF
plt.savefig('average_reward_plot_with_samples_and_confidence_intervals.pdf', format='pdf')

# Show plot
plt.show()
