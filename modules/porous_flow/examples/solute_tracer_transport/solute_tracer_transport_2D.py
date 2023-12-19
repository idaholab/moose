import matplotlib.pyplot as plt
import numpy as np
import math as mt

def C_a(C0,alphal,t,r,Qinj,n,b):
  C_a=[]
  for i in range(1,len(t)):
    R_inj=mt.sqrt((Qinj*t[i])/(mt.pi*b*n))
    C_a.append(0.5*C0*(mt.erfc((r**2 - R_inj**2)/mt.sqrt(16*alphal*(R_inj**3)/3))))
  return C_a




# Read MOOSE simulation data
s01 = np.genfromtxt('solute_tracer_transport_2D_0.1.csv', delimiter = ',', names = True, dtype = float)
s03 = np.genfromtxt('solute_tracer_transport_2D_0.3.csv', delimiter = ',', names = True, dtype = float)
s05 = np.genfromtxt('solute_tracer_transport_2D_0.5.csv', delimiter = ',', names = True, dtype = float)
s07 = np.array(np.genfromtxt('solute_tracer_transport_2D_0.7.csv', delimiter = ',', names = True, dtype = float))
s1 = np.genfromtxt('solute_tracer_transport_2D_1.csv', delimiter = ',', names = True, dtype = float)
s3 = np.genfromtxt('solute_tracer_transport_2D_3.csv', delimiter = ',', names = True, dtype = float)
s5 = np.array(np.genfromtxt('solute_tracer_transport_2D_5.csv', delimiter = ',', names = True, dtype = float))
s7 = np.genfromtxt('solute_tracer_transport_2D_7.csv', delimiter = ',', names = True, dtype = float)
s10 = np.genfromtxt('solute_tracer_transport_2D_10.csv', delimiter = ',', names = True, dtype = float)

C0=0.001
r=25
Qinj=0.0002
n=0.25
b=1

alphal=0.1
t=s01['time']
C_a_01=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=0.3
t=s03['time']
C_a_03=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=0.5
t=s05['time']
C_a_05=C_a(C0,alphal,t,r,Qinj,n,b)


alphal=0.7
t=s07['time']
C_a_07=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=1
t=s1['time']
C_a_1=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=3
t=s3['time']
C_a_3=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=5
t=s5['time']
C_a_5=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=7
t=s7['time']
C_a_7=C_a(C0,alphal,t,r,Qinj,n,b)

alphal=10
t=s10['time']
C_a_10=C_a(C0,alphal,t,r,Qinj,n,b)


ylim=[-0.0001, 1.1]
xlim=[-10, 100]

plt.figure(0)


plt.plot(s07['time']/(3600*24), np.array(s07['C'])*1e6,'ro' , markevery=5, label = 'MOOSE (dispersivity=0.7)')
plt.plot(s07['time'][1:]/(3600*24), np.array(C_a_07)*1000,'r' , markevery=5, label = 'Analytical (dispersivity=0.7)')
plt.plot(s5['time']/(3600*24), np.array(s5['C'])*1e6,'go' , markevery=5, label = 'MOOSE (dispersivity=5.0)')
plt.plot(s5['time'][1:]/(3600*24), np.array(C_a_5)*1000,'g' , markevery=5, label = 'Analytical (dispersivity=5)')

plt.xlabel('Time (Days)')
plt.ylabel('Concentration (mg/L)')
plt.legend()
plt.ylim(ylim)
plt.tight_layout()
plt.xlim(xlim)
plt.grid()
plt.savefig("concen_time_2D.png")


plt.figure(1)

def RMS(CM,Ca,RMS_plot):
  x=np.array(CM[1:])*1e6-np.array(Ca)*1e3
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
plt.plot(alpha_plot, np.array(RMS_plot),'ro' )
plt.xlabel('Dispersivity (m)')
plt.ylabel('Concentration (mg/L)')
plt.title('RMS error of MOOSE results with respect to different Dispersivity')
plt.tight_layout()
plt.ylim([0,0.07])
plt.grid()
plt.savefig("RMS_tracer_2D.png")
