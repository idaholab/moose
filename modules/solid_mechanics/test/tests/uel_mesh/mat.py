#%%
# imports
import matplotlib.pyplot as plt
import numpy as np

# %%
def read_matrix(fname):
  with open(fname) as f:
    lines = [l.strip().split(':') for l in f.readlines() if l[:3] == 'row']
  n = len(lines)
  mat = np.zeros((n,n))
  for l in lines:
    row = int(l[0][4:])
    for item in l[1].split('  '):
      pair = item.strip(' ()').split(',')
      col = int(pair[0])
      val = float(pair[1])
      mat[row][col] = val
  return mat

# %%
fd = read_matrix('mat_fd.dat')
nofd = read_matrix('mat_nofd.dat')

#%%
plt.imshow(fd-nofd)
plt.colorbar()

# %%
