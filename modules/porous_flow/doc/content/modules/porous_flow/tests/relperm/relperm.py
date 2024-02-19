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

def effS(S_res,sum_S_res,S): #this function calculate the effective saturation

  eff_S=[]

  for i in range(len(S)):
    num=S[i]-S_res #numerator
    den=1-sum_S_res #denominator
    a=num/den
    if a>1:
      a=1
    elif a<0:
      a=0

    eff_S.append(a)
  return eff_S


def brook_corey_func(lamda,S_res_w=0,sum_S_res_w=0,S_res_nw=0,sum_S_res_nw=0): #This function return a list of 2 listes, the first one is for wet phase and second is for non wet
  k=[]
  Seff_w=effS(S_res_w,sum_S_res_w,np.linspace(0,1,100))


  k_w=[]
  for i in range(len(Seff_w)):
    a=Seff_w[i]**((2+3*lamda)/lamda)

    k_w.append(a)

  k.append(k_w)

  Seff_nw=effS(S_res_nw,sum_S_res_nw,np.linspace(0,1,100))
  k_nw=[]
  for i in range(len(Seff_nw)):
    a=(1-Seff_nw[i])**2
    b=(1-Seff_nw[i]**((2+lamda)/lamda))
    k_nw.append(a*b)

  k.append(k_nw)
  return k


def corey_func(n,S_res_w=0,sum_S_res_w=0,S_res_nw=0,sum_S_res_nw=0,scalew=1,scalenw=1): #This function return a list of 2 listes, the first one is for wet phase and second is for non wet
  k=[]
  Seff_w=effS(S_res_w,sum_S_res_w,np.linspace(0,1,100))


  k_w=[]
  for i in range(len(Seff_w)):
    a=((Seff_w[i])**(n))*scalew

    k_w.append(a)

  k.append(k_w)

  Seff_nw=effS(S_res_nw,sum_S_res_nw,np.linspace(1,0,100))
  k_nw=[]
  for i in range(len(Seff_nw)):
    a=((Seff_nw[i])**(n))*scalenw

    k_nw.append(a)

  k.append(k_nw)
  return k



def vanGen_func(m,S_res_w=0,sum_S_res_w=0,S_res_nw=0,sum_S_res_nw=0,scalew=1,scalenw=1): #This function return a list of 2 listes, the first one is for wet phase and second is for non wet
  k=[]
  Seff_w=effS(S_res_w,sum_S_res_w,np.linspace(0,1,100))


  k_w=[]
  for i in range(len(Seff_w)):
    a=(Seff_w[i])**0.5
    b=(1-(1-Seff_w[i]**(1/m))**m)**2

    k_w.append(a*b)

  k.append(k_w)

  Seff_nw=effS(S_res_nw,sum_S_res_nw,np.linspace(1,0,100))
  k_nw=[]
  for i in range(len(Seff_nw)):
    a=(Seff_nw[i])**0.5
    b=(1-(1-Seff_nw[i])**(1/m))**(2*m)
    k_nw.append(a*b)

  k.append(k_nw)
  return k



S=np.linspace(0,1,100) # Saturation of phase 0
S1=np.linspace(1,0,100) # Saturation of phase 1


# Read MOOSE simulation data
brook_corey1 = np.genfromtxt('../../../../../../test/tests/relperm/brooks_corey1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
brook_corey2 = np.genfromtxt('../../../../../../test/tests/relperm/brooks_corey2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)

corey1 = np.genfromtxt('../../../../../../test/tests/relperm/corey1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
corey2 = np.genfromtxt('../../../../../../test/tests/relperm/corey2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
corey3 = np.genfromtxt('../../../../../../test/tests/relperm/corey3_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
corey4 = np.genfromtxt('../../../../../../test/tests/relperm/corey4_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)

vangen1 = np.genfromtxt('../../../../../../test/tests/relperm/vangenuchten1_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)
vangen2 = np.genfromtxt('../../../../../../test/tests/relperm/vangenuchten2_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)

unity=np.genfromtxt('../../../../../../test/tests/relperm/unity_out_vpp_0001.csv', delimiter = ',', names = True, dtype = float)


## Adjust the setting here
x_lim=[-0.05,1.05]
y_lim=[-0.05,1.05]


plt.figure(0)
# Brook Corey 1
brook_corey_a1=brook_corey_func(2) #analytical
plt.plot(S, brook_corey_a1[0] ,'b' , label = 'kr0')
plt.plot(S, brook_corey_a1[1] , 'g' ,label = 'kr1')
plt.plot(brook_corey1['s0aux'], brook_corey1['kr0aux'],'ob' , label = 'kr0 (MOOSE)')
plt.plot(brook_corey1['s0aux'], brook_corey1['kr1aux'],'og' , label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Brooks-Corey relative permeability: $\lambda =2$')
plt.tight_layout()
plt.savefig("brook_corey1.png")


plt.figure(1)
# Brook Corey 2
brook_corey_a2=brook_corey_func(2,0.2,0.5,0.2,0.5) #analytical
plt.plot(S, brook_corey_a2[0] ,'b', label = 'kr0')
plt.plot(S, brook_corey_a2[1] , 'g',label = 'kr1')
plt.plot(brook_corey2['s0aux'], brook_corey2['kr0aux'],'ob' , label = 'kr0 (MOOSE)')
plt.plot(brook_corey2['s0aux'], brook_corey2['kr1aux'] ,'og' , label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Brooks-Corey relative permeability: $S_{0r}=0.2, S_{1r}=0.3, \lambda =2$')
plt.tight_layout()
plt.savefig("brook_corey2.png")


plt.figure(2)
# Corey 1
corey_a1=corey_func(1) #analytical
plt.plot(S, corey_a1[0] ,'b' , label = 'kr0')
plt.plot(S, corey_a1[1] , 'g' ,label = 'kr1')
plt.plot(corey1['s0aux'], corey1['kr0aux'] ,'bo'  , label = 'kr0 (MOOSE)')
plt.plot(corey1['s0aux'], corey1['kr1aux'] ,'go'  , label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Corey relative permeability: $n=1$ ')
plt.tight_layout()
plt.savefig("corey1.png")


plt.figure(3)
# Corey 2
corey_a2=corey_func(2) #analytical
plt.plot(S, corey_a2[0] ,'b' , label = 'kr0')
plt.plot(S, corey_a2[1] , 'g' ,label = 'kr1')
plt.plot(corey2['s0aux'], corey2['kr0aux'],'bo'  , label = 'kr0 (MOOSE)')
plt.plot(corey2['s0aux'], corey2['kr1aux'],'go'  , label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Corey relative permeability: $n=2$')
plt.tight_layout()
plt.savefig("corey2.png")

plt.figure(4)
# Corey 3
corey_a3=corey_func(2,0.2,0.5,0.3,0.5) #analytical
plt.plot(S, corey_a3[0] ,'b' , label = 'kr0')
plt.plot(S, corey_a3[1] , 'g' ,label = 'kr1')
plt.plot(corey3['s0aux'], corey3['kr0aux'] ,'bo', label = 'kr0 (MOOSE)')
plt.plot(corey3['s0aux'], corey3['kr1aux'] ,'go', label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Corey relative permeability: $S_{0r}=0.2, S_{1r}=0.3, n=2$')
plt.tight_layout()
plt.savefig("corey3.png")

plt.figure(5)
# Corey 4
corey_a4=corey_func(2,0.2,0.5,0.3,0.5,0.1,10) #analytical
plt.plot(S, corey_a4[0] ,'b' , label = 'kr0')
plt.plot(S, corey_a4[1] , 'g' ,label = 'kr1')
plt.plot(corey4['s0aux'], corey4['kr0aux'] ,'bo' , label = 'kr0 (MOOSE)')
plt.plot(corey4['s0aux'], corey4['kr1aux'] ,'go' , label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Corey relative permeability: $S_{0r}=0.2, S_{1r}=0.3, n=2$, scaling applied')
plt.tight_layout()
plt.ylim([0, 10.5])
plt.savefig("corey4.png")


plt.figure(6)
# Vangenuchten 1
vanGen_a1=vanGen_func(0.5)
plt.plot(S, vanGen_a1[0] ,'b' , label = 'kr0')
plt.plot(S, vanGen_a1[1] ,'g' , label = 'kr1')
plt.plot(vangen1['s0aux'], vangen1['kr0aux'],'bo' , label = 'kr0 (MOOSE)')
plt.plot(vangen1['s0aux'], vangen1['kr1aux'] ,'go', label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Vangenuchten relative permeability: $m=0.5$')
plt.tight_layout()
plt.savefig("vangen1.png")

plt.figure(7)
# Vangenuchten 2
vanGen_a2=vanGen_func(0.4,0.1,0.3,0.2,0.3)
plt.plot(S, vanGen_a2[0] ,'b' , label = 'kr0')
plt.plot(S, vanGen_a2[1] ,'g' , label = 'kr1')
plt.plot(vangen2['s0aux'], vangen2['kr0aux'] ,'bo' , label = 'kr0 (MOOSE)')
plt.plot(vangen2['s0aux'], vangen2['kr1aux'] ,'go' ,label = 'kr1 (MOOSE)')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('van Genuchten relative permeability: $S_{0r}=0.2, S_{1r}=0.3, m=0.4$')
plt.tight_layout()
plt.savefig("vangen2.png")


plt.figure(8)
# unity
plt.plot(unity['s0aux'], unity['kr0aux'] , label = 'kr0')
plt.plot(unity['s0aux'], unity['kr1aux'] , label = 'kr1')
plt.xlim(x_lim)
plt.ylim(y_lim)
plt.xlabel('Phase 0 saturation (-)')
plt.ylabel('Relative permeability (-)')
plt.legend()
plt.title('Unity relative permeability Test 2')
plt.tight_layout()
plt.savefig("unity.png")
