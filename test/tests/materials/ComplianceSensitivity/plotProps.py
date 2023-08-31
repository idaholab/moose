import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

# -------------------ParsedFunctionOutput

df = pd.read_csv("orderedSimp2MatTest_out.csv")
print("description: ", df.columns.values)

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.set_title("Plot title")
ax1.set_xlabel("density")
ax1.set_ylabel("E")
ax1.plot(df["mat_den"], df["E_phys"], c="r", label="E")
leg = ax1.legend()

# -------------------Analytical

E0 = 1e-5
E1 = 0.6
E2 = 1.0

rho0 = 0.0
rho1 = 0.4
rho2 = 1.0

simpExp = 2

mat_dens1 = np.linspace(rho0, rho1, 20)
A1 = (E0 - E1) / (rho0**simpExp - rho1**simpExp)
B1 = E0 - A1 * rho0**simpExp
Ee1 = A1 * np.power(mat_dens1, simpExp) + B1

mat_dens2 = np.linspace(rho1, rho2, 20)
A2 = (E1 - E2) / (rho1**simpExp - rho2**simpExp)
B2 = E1 - A2 * rho1**simpExp
Ee2 = A2 * np.power(mat_dens2, simpExp) + B2


fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.set_title("Plot title")
ax1.set_xlabel("density")
ax1.set_ylabel("E")

ax1.plot(mat_dens1, Ee1, c="r", label="E1")
ax1.plot(mat_dens2, Ee2, c="b", label="E2")
leg = ax1.legend()

plt.show()
