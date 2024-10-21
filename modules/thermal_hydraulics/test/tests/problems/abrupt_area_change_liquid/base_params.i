t_end = 1e-3
CFL = 0.8

gamma = 2.25
p_inf = 1e9
q = 0
cv = 2700

lengthL = 3.0
lengthR = 2.0
x_disc = 2.0
xR = ${lengthL}

NL = 600
NR = 400

pL = 1e6
pR = 1e5

TL = 296.9971026 # rho = 998.638 kg/m^3
TR = 296.8519476 # rho = 998.228 kg/m^3

DL = 2e-2
DR = 1e-2
AL = ${fparse 0.25 * pi * DL^2}
AR = ${fparse 0.25 * pi * DR^2}
