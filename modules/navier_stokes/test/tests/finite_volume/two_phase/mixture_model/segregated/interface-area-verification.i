##########################################################
# Verification of both interfacial area concentration closure sets against their steady states.
#
# The models implemented are those of the ANSYS Fluent Theory Guide section 16.4.9, whose transport
# equation, 16.4-27, is
#
#   d(rho_g xi)/dt + div(rho_g u_g xi)
#     = (1/3)(D rho_g/Dt) xi + (2/3)(mdot_g/alpha_g) xi + rho_g (S_RC + S_WE + S_TI)
#
# Here there is no flow, the dispersed phase density is constant and there is no mass transfer, so
# the advection, the expansion term and the phase change term all vanish identically and the
# equation reduces to the relaxation
#
#   d(xi)/dt = S_RC + S_WE + S_TI .
#
# Its steady state is the balance of coalescence against breakage, S_RC + S_WE + S_TI = 0, which is
# a scalar equation in the particle size d_b = psi alpha_g / xi alone: every term of both models
# carries the same power of xi, so xi divides out and only the d_b dependence of u_t, We, u_r and
# the exponentials survives. That makes the steady state a property of the closure formulas and not
# of the discretisation, which is what makes it worth asserting.
#
# The roots below were obtained independently with a bracketing solver at machine tolerance:
#
#   Hibiki-Ishii   d_b = 2.26455455763076e-3   xi = 1.32476384368442e+3
#   Ishii-Kim      d_b = 5.86671279526857e-3   xi = 5.11359615629977e+2
#
# Two more registered tests override every coefficient of each model at once, so that a coefficient
# read into the wrong place cannot pass. Their roots, from the same solver:
#
#   Hibiki-Ishii   gamma_c = 0.15, k_c = 0.2, gamma_b = 0.3, k_b = 1.0, shape_factor = 4
#                  d_b = 1.38301818375610e-3   xi = 1.44611258441177e+3
#   Ishii-Kim      c_rc = 0.006, c_we = 0.004, c_ti = 0.1, c = 2, we_cr = 4, fd_max = 0.7
#                  d_b = 7.28295014804870e-3   xi = 4.11920985179856e+2   (We/We_cr = 4.41)
#
# The packing limit does not move the Hibiki-Ishii root, since it multiplies both of that model's
# terms alike, and the shape factor moves neither root: it converts the size the balance fixes into
# an interfacial area concentration, which is why it is varied on the Hibiki-Ishii case.
#
# The Ishii-Kim case sits at We/We_cr = 2.05. That matters: the breakage source carries a factor
# sqrt(1 - We_cr/We) whose derivative is infinite at the critical Weber number, so a balance landing
# near the cutoff would be ill conditioned and would test the solver rather than the model. The
# phase fraction of 0.5 was chosen to move it clear of that point.
##########################################################

rho_g = 1.2
rho_f = 1000.0
mu_f = 1.0e-3
alpha = 0.5
sigma = 0.072
epsilon = 10.0
model = 'hibiki-ishii'
shape_factor = 6.0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Problem]
  linear_sys_names = 'xi_system'
[]

[Variables]
  [interface_area]
    type = MooseLinearVariableFVReal
    solver_sys = xi_system
    # Started well away from both steady states, and on the same side of each
    initial_condition = 200.0
  []
[]

[LinearFVKernels]
  # d(rho_g xi)/dt, the conservative time derivative of equation 16.4-27
  [time]
    type = LinearFVTimeDerivative
    variable = interface_area
    factor = ${rho_g}
  []
  [sources]
    type = LinearWCNSFV2PInterfaceAreaSourceSink
    variable = interface_area
    model = ${model}
    u = 0
    v = 0
    rho_d = ${rho_g}
    rho_f = ${rho_f}
    mu_f = ${mu_f}
    fraction_dispersed = ${alpha}
    sigma = ${sigma}
    epsilon = ${epsilon}
    shape_factor = ${shape_factor}
    gravity = '0 -9.81 0'
  []
[]

[Executioner]
  type = Transient
  system_names = 'xi_system'
  # Long enough that both balances are converged to machine precision, not merely approached
  start_time = 0
  end_time = 500
  dt = 1
  l_tol = 1e-12
  l_abs_tol = 1e-14
[]

[Postprocessors]
  [xi]
    type = ElementAverageValue
    variable = interface_area
    execute_on = 'TIMESTEP_END'
  []
  # The particle size the steady state is really a statement about
  [d_b]
    type = ParsedPostprocessor
    expression = '${shape_factor} * ${alpha} / xi'
    pp_names = 'xi'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
