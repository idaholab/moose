import os
import matplotlib.pyplot as plt
import pandas

os.chdir(os.path.dirname(os.path.realpath(__file__)))

data = pandas.read_csv('example_data.csv')

plt.xlabel("x")
plt.ylabel("y")
plt.plot(data['x'], data['y'])
plt.savefig('example_plot.png')
