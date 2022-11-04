# Method of manufactured solutions (MMS) problem for 1-phase flow model.
#
# The python script mms_derivation.py derives the MMS sources used in this
# input file.
#
# To perform a convergence study, run this input file with different values of
# 'refinement_level', starting with 0. Manually create a CSV file (call it the
# "convergence CSV file") to store the error vs. mesh size data. It should have
# the columns specified in the plot script plot_convergence_1phase.py. Copy the
# CSV output from each run into the convergence CSV file. After all of the runs,
# run the plot script using python.

refinement_level = 0 # 0 is initial
n_elems_coarse = 10
n_elems = ${fparse int(n_elems_coarse * 2^refinement_level)}

dt = 1e-6
t_end = ${fparse dt * 10}

area = 1.0
gamma = 2.0
M = 0.05
A = 1
B = 1
C = 1

aA = ${fparse area}

R_univ = 8.3144598
R = ${fparse R_univ / M}
cp = ${fparse gamma * R / (gamma - 1.0)}
cv = ${fparse cp / gamma}

[GlobalParams]
  gravity_vector = '0 0 0'

  closures = simple_closures
[]

[Functions]
  # solutions
  [rho_fn]
    type = ParsedFunction
    expression = 'A * (sin(B*x + C*t) + 2)'
    symbol_names = 'A B C'
    symbol_values = '${A} ${B} ${C}'
  []
  [vel_fn]
    type = ParsedFunction
    expression = 'A * t * sin(pi * x)'
    symbol_names = 'A'
    symbol_values = '${A}'
  []
  [p_fn]
    type = ParsedFunction
    expression = 'A * (cos(B*x + C*t) + 2)'
    symbol_names = 'A B C'
    symbol_values = '${A} ${B} ${C}'
  []
  [T_fn]
    type = ParsedFunction
    expression = '(cos(B*x + C*t) + 2)/(cv*(gamma - 1)*(sin(B*x + C*t) + 2))'
    symbol_names = 'B C gamma cv'
    symbol_values = '${B} ${C} ${gamma} ${cv}'
  []

  # MMS sources
  [rho_src_fn]
    type = ParsedFunction
    expression = 'A^2*B*t*sin(pi*x)*cos(B*x + C*t) + pi*A^2*t*(sin(B*x + C*t) + 2)*cos(pi*x) + A*C*cos(B*x + C*t)'
    symbol_names = 'A B C'
    symbol_values = '${A} ${B} ${C}'
  []
  [rhou_src_fn]
    type = ParsedFunction
    expression = 'A^3*B*t^2*sin(pi*x)^2*cos(B*x + C*t) + 2*pi*A^3*t^2*(sin(B*x + C*t) + 2)*sin(pi*x)*cos(pi*x) + A^2*C*t*sin(pi*x)*cos(B*x + C*t) + A^2*(sin(B*x + C*t) + 2)*sin(pi*x) - A*B*sin(B*x + C*t)'
    symbol_names = 'A B C'
    symbol_values = '${A} ${B} ${C}'
  []
  [rhoE_src_fn]
    type = ParsedFunction
    expression = 'A*C*(A^2*t^2*sin(pi*x)^2/2 + (cos(B*x + C*t) + 2)/((gamma - 1)*(sin(B*x + C*t) + 2)))*cos(B*x + C*t) + pi*A*t*(A*(A^2*t^2*sin(pi*x)^2/2 + (cos(B*x + C*t) + 2)/((gamma - 1)*(sin(B*x + C*t) + 2)))*(sin(B*x + C*t) + 2) + A*(cos(B*x + C*t) + 2))*cos(pi*x) + A*t*(A*B*(A^2*t^2*sin(pi*x)^2/2 + (cos(B*x + C*t) + 2)/((gamma - 1)*(sin(B*x + C*t) + 2)))*cos(B*x + C*t) - A*B*sin(B*x + C*t) + A*(sin(B*x + C*t) + 2)*(pi*A^2*t^2*sin(pi*x)*cos(pi*x) - B*sin(B*x + C*t)/((gamma - 1)*(sin(B*x + C*t) + 2)) - B*(cos(B*x + C*t) + 2)*cos(B*x + C*t)/((gamma - 1)*(sin(B*x + C*t) + 2)^2)))*sin(pi*x) + A*(sin(B*x + C*t) + 2)*(A^2*t*sin(pi*x)^2 - C*sin(B*x + C*t)/((gamma - 1)*(sin(B*x + C*t) + 2)) - C*(cos(B*x + C*t) + 2)*cos(B*x + C*t)/((gamma - 1)*(sin(B*x + C*t) + 2)^2))'
    symbol_names = 'A B C gamma'
    symbol_values = '${A} ${B} ${C} ${gamma}'
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = ${gamma}
    molar_mass = ${M}
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase

    fp = fp

    # geometry
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1.0
    n_elems = ${n_elems}
    A = ${area}

    # IC
    initial_p = p_fn
    initial_T = T_fn
    initial_vel = 0

    f = 0
  []

  [left_boundary]
    type = InletFunction1Phase
    input = 'pipe:in'
    p = p_fn
    rho = rho_fn
    vel = vel_fn
  []

  [right_boundary]
    type = InletFunction1Phase
    input = 'pipe:out'
    p = p_fn
    rho = rho_fn
    vel = vel_fn
  []
[]

[Kernels]
  [rho_src]
    type = BodyForce
    variable = rhoA
    function = rho_src_fn
    value = ${aA}
  []
  [rhou_src]
    type = BodyForce
    variable = rhouA
    function = rhou_src_fn
    value = ${aA}
  []
  [rhoE_src]
    type = BodyForce
    variable = rhoEA
    function = rhoE_src_fn
    value = ${aA}
  []
[]

[Postprocessors]
  [rho_err]
    type = ElementL1Error
    variable = rho
    function = rho_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vel_err]
    type = ElementL1Error
    variable = vel
    function = vel_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [p_err]
    type = ElementL1Error
    variable = p
    function = p_fn
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 3
  []

  start_time = 0
  dt = ${dt}
  end_time = ${t_end}
  abort_on_solve_fail = true

  [Quadrature]
    type = GAUSS
    order = FIRST
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
  velocity_as_vector = false
[]
