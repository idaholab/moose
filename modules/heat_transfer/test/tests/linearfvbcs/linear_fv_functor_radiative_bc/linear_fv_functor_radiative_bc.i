# 1D steady-state heat conduction with a radiative BC on the right boundary.
# The domain is [0,1] with:
#   Left:  Dirichlet temperature T_L (fixed)
#   Right: LinearFVFunctorRadiativeBC, q = sigma * eps * (T^4 - Tinf^4)
# A pseudo-transient approach (large dt steps) drives the Picard iteration
# so that the Newton linearization converges to the nonlinear solution.

T_L   = 1000.0   # Left boundary temperature [K]
T_inf = 300.0    # Far-field radiation temperature [K]
eps   = 1.0      # Emissivity
k     = 1.0      # Thermal conductivity [W/(m·K)]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
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
  [T_avg]
    type = ElementAverageValue
    variable = T
  []
  [T_max]
    type = ElementExtremeValue
    variable = T
    value_type = max
  []
  [T_min]
    type = ElementExtremeValue
    variable = T
    value_type = min
  []
[]

[Executioner]
  type = Transient
  system_names = heat_system
  scheme = 'implicit-euler'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 1e6
  num_steps = 50
  l_tol = 1e-10
[]

[Outputs]
  file_base = 'linear_fv_functor_radiative_bc_hot_body_out'
  csv = true
  execute_on = final
[]
