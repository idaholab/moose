# -----------------------------------------------------------------------------
# Geometry parameters
# -----------------------------------------------------------------------------
dia = ${fparse 0.5*2.54/100.0}
circle_radius = ${fparse 0.5*dia}
pitch = ${fparse 4*dia}
x_min = -0.2
x_max = 1.2
y_min = ${fparse -0.5*9.82*2.54/100.0}
y_max = ${fparse  0.5*9.82*2.54/100.0}
rundoff = 1e-4
sectors = 32
# -----------------------------------------------------------------------------
# Material properties
# -----------------------------------------------------------------------------
cp_f  = 1010.0
mu    = 1.82e-5
Pr    = 0.713
k_f   = ${fparse cp_f*mu/Pr}
rho_f = 1.176703752 # https://srd.nist.gov/jpcrdreprint/1.1285884.pdf 

cp_s  = 390.0
rho_s = 8940
k_s   = 401.0
# -----------------------------------------------------------------------------
# Boundary conditions
# -----------------------------------------------------------------------------
Re = 172
#inlet_velocity = ${fparse Re*mu/rho/dia}
v_in = ${fparse Re*mu/rho_f/dia}
P_out = 0

h_s   = 0.1
T_s   = 312.0

T_f   = 300.0

# source calc
#V_s = ${fparse pi* (dia^3)/4.0}
#A_f = ${fparse 4.65*0.0254 * dia}
#q_s = ${fparse cp_f*T_f*A_f*mu*Re/V_s/dia}
Nu = 2.8
q_s = ${fparse 0.0*Nu*(T_s - T_f) * k_f / (circle_radius^2)}
# -----------------------------------------------------------------------------
# Numerical schemes
# -----------------------------------------------------------------------------
dt = 0.001
t_end = 4.000
#advected_interp_method = 'upwind'
advected_interp_method = 'average'
