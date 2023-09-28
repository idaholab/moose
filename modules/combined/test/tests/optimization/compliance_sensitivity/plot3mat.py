import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# -------------------Plotting objective function
df = pd.read_csv("paper_three_materials_test_out.csv")
print("description: ", df.columns.values)

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.plot(df["time"], df["objective"], c="r", label="E")

ax1.set_title("Objective Function: Strain Energy")
ax1.set_xlabel("Iteration")
ax1.set_ylabel("Objective Function")

# -------------------Plotting Constraints
vol_frac = 0.4
cost_frac = 0.2
vol_frac_arr = [vol_frac] * df["time"].shape[0]
cost_frac_arr = [cost_frac] * df["time"].shape[0]


fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.plot(df["time"], df["vol_frac"], c="r", linestyle="-", label="Volume")
ax1.plot(df["time"], vol_frac_arr, c="r", linestyle=":", label="Volume Constraint")
ax1.plot(df["time"], df["cost_frac"], c="b", linestyle="-", label="Cost")
ax1.plot(df["time"], cost_frac_arr, c="b", linestyle=":", label="Cost Constraint")

leg = ax1.legend()

ax1.set_title("Constraints: Cost and Volume")
ax1.set_xlabel("Iteration")
ax1.set_ylabel("Constraints")

plt.show()
