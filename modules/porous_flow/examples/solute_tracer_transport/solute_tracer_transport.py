import matplotlib.pyplot as plt
import numpy as np
import math as mt

def C_a(C0,alphal,qx,t,x):
  C_a=[]
  Dl = alphal *  qx
  for i in range(len(t)):
    a=(x-qx*t[i])/(2*(Dl*t[i])**0.5)
    a=mt.erfc(a)
    b=mt.exp(qx*x/(Dl))
    c=(x+qx*t[i])/(2*(Dl*t[i])**0.5)
    c=mt.erfc(c)
    C_a.append(0.5*C0*(a+b*c))
  return C_a

# Read MOOSE simulation data
s01 = np.genfromtxt('solute_tracer_transport_0.1.csv', delimiter = ',', names = True, dtype = float)
s03 = np.genfromtxt('solute_tracer_transport_0.3.csv', delimiter = ',', names = True, dtype = float)
s05 = np.genfromtxt('solute_tracer_transport_0.5.csv', delimiter = ',', names = True, dtype = float)
s07 = np.genfromtxt('solute_tracer_transport_0.7.csv', delimiter = ',', names = True, dtype = float)
s1 = np.genfromtxt('solute_tracer_transport_1.csv', delimiter = ',', names = True, dtype = float)
s3 = np.genfromtxt('solute_tracer_transport_3.csv', delimiter = ',', names = True, dtype = float)
s5 = np.genfromtxt('solute_tracer_transport_5.csv', delimiter = ',', names = True, dtype = float)
s7 = np.genfromtxt('solute_tracer_transport_7.csv', delimiter = ',', names = True, dtype = float)
s10 = np.genfromtxt('solute_tracer_transport_10.csv', delimiter = ',', names = True, dtype = float)



# Producing analytical solution
C0=0.001
x=50
phi = 0.25 # Porosity
qx = 1e-4 * (12 - 10) / 100 / phi # q = K (h1-h2)/L / phi

alphaL=0.1
t=s01['time']
C_a_01=C_a(C0,alphaL,qx,t,x)

alphaL=0.3
t=s03['time']
C_a_03=C_a(C0,alphaL,qx,t,x)

alphaL=0.5
t=s05['time']
C_a_05=C_a(C0,alphaL,qx,t,x)

alphaL=0.7
t=s07['time']
C_a_07=C_a(C0,alphaL,qx,t,x)

alphaL=1
t=s1['time']
C_a_1=C_a(C0,alphaL,qx,t,x)

alphaL=3
t=s3['time']
C_a_3=C_a(C0,alphaL,qx,t,x)

alphaL=5
t=s5['time']
C_a_5=C_a(C0,alphaL,qx,t,x)

alphaL=7
t=s7['time']
C_a_7=C_a(C0,alphaL,qx,t,x)

alphaL=10
t=s10['time']
C_a_10=C_a(C0,alphaL,qx,t,x)



ylim=[-0.0001, 1.0001]
xlim=[0, 140]

plt.figure(0)

plt.plot(np.array(s07['time'])/(3600*24), np.array(s07['C'])*1000,'ro' , markevery=10, label = 'MOOSE (dispersivity=0.7)')
plt.plot(np.array(s07['time'])/(3600*24), np.array(C_a_07)*1000,'r' , label = 'Analytical (dispersivity=0.7)')
plt.plot(np.array(s5['time'])/(3600*24), np.array(s5['C'])*1000,'bo' , markevery=10, label = 'MOOSE (dispersivity=5.0)')
plt.plot(np.array(s5['time'])/(3600*24), np.array(C_a_5)*1000,'b', label = 'Analytical (dispersivity=5.0)')

plt.grid()
plt.xlabel('Time (Days)')
plt.ylabel('Concentration (mg/L)')
plt.legend()
plt.ylim(ylim)
plt.tight_layout()
plt.xlim(xlim)
plt.savefig("concen_time.png")



plt.figure(1)

def RMS(CM,Ca,RMS_plot):
  x=np.array(CM[1:])-np.array(Ca[1:]) #ignore the first Nan element
  RMS_plot.append(np.sqrt(np.mean(x**2)))
  return RMS_plot

RMS_plot=[]
RMS(s01['C'],C_a_01,RMS_plot)
RMS(s03['C'],C_a_03,RMS_plot)
RMS(s05['C'],C_a_05,RMS_plot)
RMS(s07['C'],C_a_07,RMS_plot)
RMS(s1['C'],C_a_1,RMS_plot)
RMS(s3['C'],C_a_3,RMS_plot)
RMS(s5['C'],C_a_5,RMS_plot)
RMS(s7['C'],C_a_7,RMS_plot)
RMS(s10['C'],C_a_10,RMS_plot)

alpha_plot=[0.1,0.3,0.5,0.7,1,3,5,7,10]
plt.plot(alpha_plot, np.array(RMS_plot)*1000,'ro' )
plt.xlabel('Dispersivity (m)')
plt.ylabel('Concentration (mg/L)')
plt.title('RMS error of MOOSE result with respect to different Dispersivity')
plt.tight_layout()
plt.grid()
plt.savefig("RMS_tracer.png")
