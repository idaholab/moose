# MMS convergence test for LinearFVFunctorRadiativeBC.
#
# Exact solution: T_exact(x) = T_L + B * (exp(x) - 1)
# where B = (T_R_mms - T_L) / (e - 1).
#
# This satisfies:
#   Left BC:  T(0) = T_L                          (Dirichlet)
#   Right BC: -k * T'(1) = sigma*eps*(T_R_mms^4 - T_inf^4)  (radiative)
#             with T'(1) = B*e = (T_R_mms - T_L)*e/(e-1)
#             i.e. k*(T_L - T_R_mms)*e/(e-1) = sigma*eps*(T_R_mms^4 - T_inf^4)
#   Source:   f(x) = -k * T_exact'' = -k * B * exp(x)  (spatially varying)
#
# The exponential profile produces non-trivial interior discretization error,
# ensuring the convergence test exercises both the FV interior scheme and the
# Robin boundary condition simultaneously.
#
# T_R_mms must be supplied via CLI args, as the root of:
#   k*(T_L - T_R)*e/(e-1) = sigma*eps*(T_R^4 - T_inf^4)
# (computed in the Python convergence script).

T_L    = 1000.0
T_inf  = 300.0
k      = 1.0
eps    = 1.0
T_R_mms = 0  # set via CLI: T_R_mms=<value>
B_mms = '${fparse (T_R_mms - T_L) / (exp(1.0) - 1.0)}'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10  # overridden by CLI: Mesh/gen/nx=<N>
    xmin = 0
    xmax = 1
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
    expression = 'T_L + B_mms * (exp(x) - 1.0)'
    symbol_names = 'T_L B_mms'
    symbol_values = '${T_L} ${B_mms}'
  []
  [source_fn]
    type = ParsedFunction
    expression = '-k * B_mms * exp(x)'
    symbol_names = 'k B_mms'
    symbol_values = '${k} ${B_mms}'
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
    Tinfinity = ${T_inf}
    diffusion_coeff = ${k}
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
