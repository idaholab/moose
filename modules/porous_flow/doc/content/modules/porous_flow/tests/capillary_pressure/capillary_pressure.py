#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib.pyplot as plt
import numpy as np

## creating the analytical function

def effS(S1_res,S): #this function calculate the effective saturation

  eff_S=np.array((S-S1_res)/(1-S1_res))



  return eff_S


def brook_corey_func(lamda,Pe,S1_ref): #This function return a list of 2 listes, the first one is for wet phase and second is for non wet
  Seff=effS(S1_ref,np.linspace(0,1,100))
  Pc=Pe*Seff**(-1/lamda)

  return Pc




def vanGen_func(m,alpha,S1_ref,scale=1,S=np.linspace(0,1,100)): #This function return a list of 2 listes, the first one is for wet phase and second is for non wet
  Seff=effS(S1_ref,S)
  Pc=[]
  for i in range(len(Seff)):
    if Seff[i]>=1:
      Pc.append(0)
    else:
      a=((Seff[i]*scale)**(-1/m)-1)**(1-m)/alpha
      b=((scale)**(-1/m)-1)**(1-m)/alpha
      Pc.append(a-b)
  return Pc





# Read MOOSE simulation data
brook_corey1 = np.genfromtxt('../../../../../../test/tests/capillary_pressure/brooks_corey1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
brook_corey2 = np.genfromtxt('../../../../../../test/tests/capillary_pressure/brooks_corey2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)

vangen1 = np.genfromtxt('../../../../../../test/tests/capillary_pressure/vangenuchten1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
vangen2 = np.genfromtxt('../../../../../../test/tests/capillary_pressure/vangenuchten2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
vangen3 = np.genfromtxt('../../../../../../test/tests/capillary_pressure/vangenuchten3_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)




S=np.linspace(0,1,100)
ylim=[0, 2e6]

plt.figure(0)
# Brook Corey
brook_corey_a1=brook_corey_func(2,1e5,0.1) #analytical
brook_corey_a2=brook_corey_func(2,1e5,0.1) #analytical
plt.semilogy(S, brook_corey_a1 ,'g' , label = 'Pc (Analytical)')
plt.semilogy(brook_corey1['s0aux'], np.array(brook_corey1['p1aux']) - np.array((brook_corey1['p0aux'])),'og' , markevery=15, label = 'Pc (Moose) (log_extension off)')
plt.semilogy(brook_corey2['s0aux'], np.array(brook_corey2['p1aux']) - np.array((brook_corey2['p0aux'])),'ob' , markevery=20, label = 'Pc (Moose) (log_extension on)')

plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Capillary Pressure (Pa)')
plt.legend()
plt.ylim(ylim)
plt.title('Brooks-Corey Capillary pressure: $\lambda =2, P_{e}=1*10^{5}Pa, S_{1,Res}=0.1$')
plt.tight_layout()
plt.savefig("brook_corey.png")


plt.figure(1)
ylim=[1e3, 5e8]
xlim=[-0.001,1]
# Vangenuchten 1
vanGen_a1=vanGen_func(0.5,1e-5,0.1)

plt.semilogy(S, vanGen_a1 ,'b' , label = 'Pc m=0.5')

vanGen_a3=vanGen_func(0.5,1e-5,0.1,0.8)
plt.semilogy(S, vanGen_a3 ,'r' , label = 'Pc m=0.5, scale factor=0.8')


plt.semilogy(vangen1['s0aux'], np.array(vangen1['p1aux']) - np.array((vangen1['p0aux'])),'bo', markevery=40, label = 'Pc m=0.5 (MOOSE) (log_extension off)')
plt.semilogy(vangen2['s0aux'], np.array(vangen2['p1aux']) - np.array((vangen2['p0aux'])),'go', markevery=25, label = 'Pc m=0.5 (MOOSE) (log_extension on)')
plt.semilogy(vangen3['s0aux'], np.array(vangen3['p1aux']) - np.array((vangen3['p0aux'])),'ro', markevery=60, label = 'Pc m=0.5, scale factor=0.8 (MOOSE)')
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Capillary Pressure (Pa)')
plt.ylim(ylim)
plt.xlim(xlim)
plt.legend()
plt.title('van Genuchten Capillary pressure: $ \\alpha= 1*10^{-5}, S_{1,Res}=0.1$',fontsize=11)
plt.tight_layout()
plt.savefig("vangen.png")

print('Image has been printed')
