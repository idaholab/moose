import matplotlib.pyplot as plt
import numpy as np
import math as mt

def CO2_molar_frac(mass_frac_CH4):
  M_CO2=44
  M_CH4=16
  mass_frac_CO2=1-mass_frac_CH4
  molar_CO2=(mass_frac_CO2/M_CO2)/((mass_frac_CO2/M_CO2)+(mass_frac_CH4/M_CH4))
  return molar_CO2


# Read MOOSE simulation data
year_100 = np.genfromtxt('CO2_CH4_vertical_low_diff_3153600000_mass_frac_CH4_0328.csv', delimiter = ',', names = True, dtype = float)
year_10 = np.genfromtxt('CO2_CH4_vertical_low_diff_3153600000_mass_frac_CH4_0044.csv', delimiter = ',', names = True, dtype = float)
year_0 = np.genfromtxt('CO2_CH4_vertical_low_diff_3153600000_mass_frac_CH4_0000.csv', delimiter = ',', names = True, dtype = float)



ylim=[-100, 0]
xlim=[-0.001, 1]

plt.figure(0)
#plt.plot(CO2_molar_frac(np.array(year_0['mass_frac_CH4'])), np.array(year_0['y'])+400,'--' , markevery=10, label = 'Initial')
plt.plot(CO2_molar_frac(np.array(year_10['mass_frac_CH4'])), np.array(year_10['y'])+400,'--' , markevery=10, label = '10 years')
plt.plot(CO2_molar_frac(np.array(year_100['mass_frac_CH4'])), np.array(year_100['y'])+400,'--' , markevery=10, label = '100 years')


plt.xlabel('$(X_g)^{CO_2}$')
plt.ylabel('$Z$ (m)')
plt.legend()
plt.ylim(ylim)
plt.tight_layout()
plt.xlim(xlim)
plt.grid()
plt.savefig("CO2_molar_frac.png")


