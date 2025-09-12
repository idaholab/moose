import os, sys
import matplotlib.pyplot as plt

script_dir = os.path.dirname(os.path.realpath(__file__))
os.chdir(script_dir)

plt.xlabel("x")
plt.ylabel("y")
plt.plot([0,1,2,3,4], [0,1,4,9,16])
plt.savefig('generate_plot.png')
