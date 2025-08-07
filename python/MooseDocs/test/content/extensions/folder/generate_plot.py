import os, sys
import matplotlib.pyplot as plt

script_dir = os.path.dirname(os.path.realpath(__file__))

# Make sure that moose's PYTHONPATH is set
moose_python = os.path.abspath(os.path.join(script_dir, *(['..'] * 5)))
if moose_python not in sys.path:
    raise Exception(f'Missing MOOSE python ({moose_python}) in PYTHONPATH; path={sys.path}')

os.chdir(script_dir)

plt.xlabel("x")
plt.ylabel("y")
plt.plot([0,1,2,3,4], [0,1,4,9,16])
plt.savefig('generate_plot.png')
