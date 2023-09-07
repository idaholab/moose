import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# -------------------ParsedFunctionOutput
df = pd.read_csv("orderedSimp3MatTest_out.csv")
print("description: ", df.columns.values)

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.set_title("SIMP Properties from MOOSE")
ax1.set_xlabel("Normalized Density")
ax1.set_ylabel("Normalized E & Cost")
ax1.plot(df["mat_den"], df["E_phys"], c="r", label="E")
ax1.plot(df["mat_den"], df["Cost_mat"], c="b", label="cost")
ax1.axvspan(0, 0.2, facecolor="y", alpha=0.2)
ax1.axvspan(0.2, 0.55, facecolor="r", alpha=0.2)
ax1.axvspan(0.55, 0.85, facecolor="g", alpha=0.2)
ax1.axvspan(0.85, 1.0, facecolor="maroon", alpha=0.2)

leg = ax1.legend()

# -------------------Analytical
simpExp = 4

rho0 = 1.0e-6
rho1 = 0.4
rho2 = 0.7
rho3 = 1.0

E0 = 1.0e-6
E1 = 0.2
E2 = 0.6
E3 = 1.0

C0 = 1.0e-6
C1 = 0.5
C2 = 0.8
C3 = 1.0

rho = [rho0, rho1, rho2, rho3]
E = [E0, E1, E2, E3]
C = [C0, C1, C2, C3]

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.set_title("SIMP Properties from Python Equation")
ax1.set_xlabel("Normalized Density")
ax1.set_ylabel("Normalized E & Cost")
colors = ["b", "g", "r", "c", "m", "y"]
for i in range(len(rho) - 1):
    mat_dens = np.linspace(rho[i], rho[i + 1], 20)
    A = (E[i] - E[i + 1]) / (rho[i] ** simpExp - rho[i + 1] ** simpExp)
    B = E[i] - A * rho[i] ** simpExp
    Ee = A * np.power(mat_dens, simpExp) + B

    A = (C[i] - C[i + 1]) / (rho[i] ** (1 / simpExp) - rho[i + 1] ** (1 / simpExp))
    B = C[i] - A * rho[i] ** (1 / simpExp)
    cost = A * np.power(mat_dens, (1 / simpExp)) + B

    ax1.plot(mat_dens, Ee, color=colors[i], linestyle="-", label=("E" + str(i)))
    ax1.plot(mat_dens, cost, color=colors[i], linestyle="--", label=("Cost" + str(i)))

leg = ax1.legend()

plt.show()
