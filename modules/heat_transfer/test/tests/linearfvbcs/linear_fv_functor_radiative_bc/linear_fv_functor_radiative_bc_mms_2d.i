# 2D MMS convergence test for LinearFVFunctorRadiativeBC.
#
# Exact solution: T(x,y) = T_L + B_x*(exp(x) - 1) + B_y*x*sin(pi*y)
# where B_x = (T_R_mms - T_L) / (e - 1).
#
# The solution combines a 1D exponential profile with a sinusoidal 2D
# perturbation B_y*x*sin(pi*y).  The sinusoidal y-dependence is
# non-polynomial, ensuring the FV scheme has genuine O(h^2) truncation
# error in the y-direction.
#
# At the radiative boundary (x=1):
#   T(1,y) = T_R_mms + B_y*sin(pi*y)
#   dT/dx(1,y) = B_x*e + B_y*sin(pi*y)
#
# A spatially varying T_inf(y) is computed analytically so the radiative
# BC is satisfied exactly at every boundary face:
#   T_inf(y)^4 = T(1,y)^4 + k*dT/dx(1,y)/(sigma*eps)
# At y=0,1 this recovers the 1D far-field temperature T_inf ~ 300 K.
#
# Boundary conditions:
#   Left (x=0):   Dirichlet T = T_L             (constant, since x=0)
#   Right (x=1):  Radiative Robin with T_inf(y)  (spatially varying)
#   Bottom (y=0): Dirichlet T = T_L + B_x*(exp(x) - 1)  (sin(0)=0)
#   Top (y=1):    Dirichlet T = T_L + B_x*(exp(x) - 1)  (sin(pi)=0)
#
# Source: f(x,y) = -k*(B_x*exp(x) - B_y*pi^2*x*sin(pi*y))
#
# T_R_mms must be supplied via CLI args, as the root of:
#   k*(T_L - T_R)*e/(e-1) = sigma*eps*(T_R^4 - T_inf^4)
# (computed in the Python convergence script).

T_L    = 1000.0
k      = 1.0
eps    = 1.0
sigma  = 5.670374419e-8
B_y    = 200.0

T_R_mms = 0  # set via CLI: T_R_mms=<value>
B_x = '${fparse (T_R_mms - T_L) / (exp(1.0) - 1.0)}'
B_x_e = '${fparse B_x * exp(1.0)}'
flux_scale = '${fparse k / (sigma * eps)}'
pi_val = '${fparse pi}'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10  # overridden by CLI
    ny = 10  # overridden by CLI
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
  []
[]

[Problem]
  linear_sys_names = 'heat_system'
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = 'heat_system'
    initial_condition = ${T_L}
  []
[]

[Functions]
  [exact_T]
    type = ParsedFunction
    expression = 'T_L + Bx * (exp(x) - 1.0) + By * x * sin(pv * y)'
    symbol_names = 'T_L Bx By pv'
    symbol_values = '${T_L} ${B_x} ${B_y} ${pi_val}'
  []
  [source_fn]
    type = ParsedFunction
    expression = '-k * Bx * exp(x) + k * By * pv * pv * x * sin(pv * y)'
    symbol_names = 'k Bx By pv'
    symbol_values = '${k} ${B_x} ${B_y} ${pi_val}'
  []
  [T_inf_fn]
    type = ParsedFunction
    expression = 'pow(pow(TR + By * sin(pv * y), 4) + fs * (Bxe + By * sin(pv * y)), 0.25)'
    symbol_names = 'TR By fs Bxe pv'
    symbol_values = '${T_R_mms} ${B_y} ${flux_scale} ${B_x_e} ${pi_val}'
  []
  [bottom_top_T]
    type = ParsedFunction
    expression = 'T_L + Bx * (exp(x) - 1.0)'
    symbol_names = 'T_L Bx'
    symbol_values = '${T_L} ${B_x}'
  []
[]

[LinearFVKernels]
  [time]
    type = LinearFVTimeDerivative
    variable = T
  []
  [diffusion]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k}
  []
  [source]
    type = LinearFVSource
    variable = T
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'left'
    functor = ${T_L}
  []
  [right]
    type = LinearFVFunctorRadiativeBC
    variable = T
    boundary = 'right'
    emissivity = ${eps}
    Tinfinity = T_inf_fn
    diffusion_coeff = ${k}
  []
  [bottom]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = 'bottom'
    functor = bottom_top_T
  []
  [top]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = 'T'
    boundary = 'top'
    functor = bottom_top_T
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2FunctorError
    approximate = T
    exact = exact_T
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Transient
  system_names = heat_system
  scheme = 'implicit-euler'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1e6
  num_steps = 100
  l_tol = 1e-12
[]

[Outputs]
  csv = true
  execute_on = final
[]
