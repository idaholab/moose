import matplotlib.pyplot as plt
from thm_utilities import readMOOSEXML

data = readMOOSEXML('natural_circulation_out.xml')

z_heated_pipe = data['heated_pipe_vpp']['z'][0]
x_top_pipe = data['top_pipe_vpp']['x'][0]
z_cooled_pipe = data['cooled_pipe_vpp']['z'][0]
x_bottom_pipe = data['bottom_pipe_vpp']['x'][0]
x_all = z_heated_pipe \
  + [1.0 + i for i in x_top_pipe] \
  + [2.0 + (1.0 - i) for i in reversed(z_cooled_pipe)] \
  + [3.0 + (1.0 - i) for i in reversed(x_bottom_pipe)]

def getAllValues(var):
  y_heated_pipe = data['heated_pipe_vpp'][var][0]
  y_top_pipe = data['top_pipe_vpp'][var][0]
  y_cooled_pipe = data['cooled_pipe_vpp'][var][0]
  y_bottom_pipe = data['bottom_pipe_vpp'][var][0]

  y_cooled_pipe.reverse()
  y_bottom_pipe.reverse()

  return y_heated_pipe + y_top_pipe + y_cooled_pipe + y_bottom_pipe

def makePlot(var, y_label):
  # Get the ordered list of values
  y_all = getAllValues(var)

  # Make the plot
  plt.figure(figsize=(8, 6))
  plt.rc('text', usetex=True)
  plt.rc('font', family='sans-serif')
  ax = plt.subplot(1, 1, 1)
  ax.get_yaxis().get_major_formatter().set_useOffset(False)
  plt.xlabel("Position [m]")
  plt.ylabel(y_label)
  plt.plot(x_all, y_all, '-', color='black', marker='')
  plt.tight_layout()
  plt.savefig('natural_circulation_' + var + '.png', dpi=300)

makePlot('rho', 'Density [kg/m$^3$]')
makePlot('T', 'Temperature [K]')
makePlot('p', 'Pressure [Pa]')
makePlot('rhouA', 'Mass Flow Rate [kg/s]')
