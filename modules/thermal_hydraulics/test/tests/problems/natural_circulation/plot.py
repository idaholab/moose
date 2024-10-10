import matplotlib.pyplot as plt
from thm_utilities import readMOOSEXML

data_1to1 = readMOOSEXML('junction_one_to_one.xml')
data_volume = readMOOSEXML('volume_junction.xml')

def getXValues(data):
  z_heated_pipe = data['heated_pipe_vpp']['z'][0]
  x_top_pipe = data['top_pipe_vpp']['x'][0]
  z_cooled_pipe = data['cooled_pipe_vpp']['z'][0]
  x_bottom_pipe = data['bottom_pipe_vpp']['x'][0]
  x_all = z_heated_pipe \
    + [1.0 + i for i in x_top_pipe] \
    + [2.0 + (1.0 - i) for i in reversed(z_cooled_pipe)] \
    + [3.0 + (1.0 - i) for i in reversed(x_bottom_pipe)]
  return x_all

def getYValues(data, var):
  y_heated_pipe = data['heated_pipe_vpp'][var][0]
  y_top_pipe = data['top_pipe_vpp'][var][0]
  y_cooled_pipe = data['cooled_pipe_vpp'][var][0]
  y_bottom_pipe = data['bottom_pipe_vpp'][var][0]

  y_cooled_pipe.reverse()
  y_bottom_pipe.reverse()

  return y_heated_pipe + y_top_pipe + y_cooled_pipe + y_bottom_pipe

def plotSet(data, var, color, linestyle, label):
  x = getXValues(data)
  y = getYValues(data, var)
  plt.plot(x, y, linestyle=linestyle, color=color, marker='', label=label)

def makePlot(var, y_label):
  plt.figure(figsize=(8, 6))
  plt.rc('text', usetex=True)
  plt.rc('font', family='sans-serif')
  ax = plt.subplot(1, 1, 1)
  ax.get_yaxis().get_major_formatter().set_useOffset(False)
  plt.xlabel("Position [m]")
  plt.ylabel(y_label)
  plotSet(data_1to1, var, 'black', '-', "1-to-1 junction")
  plotSet(data_volume, var, 'red', '--', "Volume junction")
  plt.tight_layout()
  plt.savefig('natural_circulation_' + var + '.png', dpi=300)

makePlot('rho', 'Density [kg/m$^3$]')
makePlot('T', 'Temperature [K]')
makePlot('p', 'Pressure [Pa]')
makePlot('rhouA', 'Mass Flow Rate [kg/s]')
