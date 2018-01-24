%This file reads in .csv from tests to ensure that they match the
%analytical model.  
clear
close all

%***Verify result for circle*************
fname1='out.csv';
Q=0.23; %eV
M0=2.5e-6; %m^4/Js
sigma=0.708; %J/m^2
T=500; %K
kb = 8.617343e-5;
lscale=1.0e-9;
tscale=1.0e-9;

mob = M0*exp(-Q/(kb*T));
Mstar = mob*sigma;

dmp=importdata(fname1);
time=dmp.data(:,1);
grarea=dmp.data(:,3);
p=polyfit(time,grarea,1);
fitvls=polyval(p,time);

slope=p(1)*lscale^2/tscale;
ratio=slope/(-2*pi*Mstar)

aslope = -2*pi*Mstar*tscale/lscale^2;

analytical = grarea(1) + aslope*(time-time(1));

figure(1)
set(gcf,'units','inches','position',[1,1,6,4])
set(gca,'fontsize',18)
plot(time,grarea,'.',time,analytical,'--','linewidth',1.5,'markersize',22)
xlabel('Time (ns)')
ylabel('Grain area (nm^2)')
axis tight
legend('MARMOT','Analytical')
legend boxoff
title('Circle grain')
%***Verify result for half loop*************
fname1='Thumb.csv';

dmp=importdata(fname1);
time=dmp.data(:,1);
grarea=dmp.data(:,2);
p=polyfit(time,grarea,1);
fitvls=polyval(p,time);

slope=p(1)*lscale^2/tscale;
ratio=slope/(-pi*Mstar)

aslope = -pi*Mstar*tscale/lscale^2;

analytical = grarea(1) + aslope*(time-time(1));

figure(2)
set(gcf,'units','inches','position',[8,1,6,4])
set(gca,'fontsize',18)
plot(time,grarea,'.',time,analytical,'--','linewidth',1.5,'markersize',22)
xlabel('Time (ns)')
ylabel('Grain area (nm^2)')
axis tight
legend('MARMOT','Analytical')
legend boxoff
title('Half loop grain')


%***Verify result for stress*************
fname1='stress.csv';

tscale = 1.0e-6;
c11_0 = 1.815e5;
c12_0 = 1.018e5;
c44_0 = 1.018e5;
dc11 = -10.9e1;
dc12 = -6.2e1;
dc44 = -5.65e1;

c11 = c11_0 + dc11*T;
c12 = c12_0 + dc12*T;
c44 = c44_0 + dc44*T;

En=[9.2 5.5]*1e6;

dmp=importdata(fname1);
time=dmp.data(:,1);
grarea=dmp.data(:,4);
GBpos=grarea/1000;

p=polyfit(time,GBpos,1);
fitvls=polyval(p,time);

slope=p(1)*lscale/tscale;
ratio = slope/(-diff(En)*mob)

figure(3)
plot(time,GBpos,'.',time,fitvls,'--')