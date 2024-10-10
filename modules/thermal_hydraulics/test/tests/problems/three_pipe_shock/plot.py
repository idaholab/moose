import matplotlib.pyplot as plt
from thm_utilities import readCSVFile

data = readCSVFile('three_pipe_shock.csv')

plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Pressure [bar]")
plt.plot(data['time'], data['p1'] / 1e5, '-', color='cornflowerblue', marker='', label='P1')
plt.plot(data['time'], data['p2'] / 1e5, '--', color='lightgreen', marker='', label='P2')
plt.plot(data['time'], data['p3'] / 1e5, ':', color='indianred', marker='', label='P3')
ax.legend(frameon=False, prop={'size':12})
plt.tight_layout()
plt.savefig('pressure.png', dpi=300)
