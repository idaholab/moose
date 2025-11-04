n_elems = 50

diam = 0.1
length = 1.0
heated_length = 0.6
power = 1e3

p_initial = 100e3
T_ambient = 300
htc = 25.0

area = ${fparse 0.25 * pi * diam^2}
S_heated = ${fparse pi * diam * heated_length}
S_cooled = ${fparse pi * diam * heated_length}

output_variables = 'rho p T vel rhouA'
