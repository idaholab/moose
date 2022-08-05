#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
  truesdell = np.loadtxt("truesdell_shear_out.csv", skiprows = 1,
      delimiter = ',')
  jaumann = np.loadtxt("jaumann_shear_out.csv", skiprows = 1,
      delimiter = ',')

  plt.plot(truesdell[:,1], truesdell[:,2], label = "Truesdell")
  plt.plot(jaumann[:,1], jaumann[:,2], label = "Jaumann")
  plt.xlabel("Shear strain")
  plt.ylabel("Shear stress")
  plt.legend(loc='best')
  plt.tight_layout()
  plt.show()
