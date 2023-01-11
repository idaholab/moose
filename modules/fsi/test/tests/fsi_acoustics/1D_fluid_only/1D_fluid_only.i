# Test for `AcousticInertia` and `Diffusion` kernels with only the fluid domain. The
# domain is 1D with length 1m and is subjected to an initial condition composed of
# a combination of sine waves. Fluid pressure is recorded at the midpoint of the
# domain. The recorded fluid pressure should match with analytical results. Because
# this implementation is equivalent to solving a 1D wave equation, analytical results
# exist.
#
# Input parameters:
# Dimensions = 1
# Length = 1 meter
# Fluid speed of sound = 1 m/s
# Initial condition = sin(pi*x) + sin(3*pi*x) + sin(5*3.141*x) + sin(7*pi*x) + sin(9*pi*x)
# Fluid domain = true
# Fluid BCs = pressures are zero on both the boundaries
# Structural domain = false

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 500
    xmax = 1
  []
[]

[GlobalParams]
[]

[Variables]
  [./p]
  [../]
[]

[Kernels]
  [./diffusion]
    type = Diffusion
    variable = 'p'
  [../]
  [./inertia]
    type = AcousticInertia
    variable = p
  [../]
[]

[BCs]
  [./leftright_pressure]
    type = DirichletBC
    variable = p
    boundary = 'left right'
    value = 0
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'p'
    function = initial_cond
  [../]
[]

[Functions]
  [./initial_cond]
    type = ParsedFunction
    expression = 'sin(pi*x) + sin(3*pi*x) + sin(5*3.141*x) + sin(7*pi*x) + sin(9*pi*x)'
  [../]
[]

[Materials]
  [./co_sq]
    type = GenericConstantMaterial
    prop_names = inv_co_sq
    prop_values = 1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  start_time = 0.0
  end_time = 1.0
  dt = 0.005
  dtmin = 0.00001
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-12
  l_tol = 1e-12
  l_max_its = 25
  timestep_tolerance = 1e-8
  automatic_scaling = true
  [TimeIntegrator]
    type = NewmarkBeta
  []
[]

[Postprocessors]
  [./p1]
    type = PointValue
    point = '0.5 0.0 0.0'
    variable = p
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
  print_linear_residuals = true
[]
