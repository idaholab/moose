# Test for `FluidFreeSurfaceBC` BC with only the fluid domain. The domain is 3D with
# lengths 1 X 1 X 0.01 meters. It is subjected to a 2D Gaussian initial condition
# with the peak at the midpoint (0.5, 0.5, 0.01). Wave heights are recorded at the
# midpoint at different times. The recorded wave heights should match with the values
# that are provided.

# Input parameters:
# Dimensions = 3
# Lengths = 1 X 1 X 0.01 meters
# Fluid speed of sound = 1500 m/s
# Initial condition = 0.00001*exp(-((x-0.5)/0.1)^2-((y-0.5)/0.1)^2)
# Fluid domain = true
# Fluid BCs = pressures are zero on all the four edges of the domain and `FluidFreeSurfaceBC` is applied on the front
# Structural domain = false

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 15
    ny = 15
    nz = 1
    xmax = 1
    ymax = 1
    zmax = 0.01
  []
[]

[GlobalParams]
[]

[Variables]
  [./p]
  [../]
[]

[AuxVariables]
  [./Wave1]
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

[AuxKernels]
  [./waves]
    type = WaveHeightAuxKernel
    variable = 'Wave1'
    pressure = p
    density = 1e-6
    gravity = 9.81
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./leftright_pressure]
    type = DirichletBC
    variable = p
    boundary = 'left right top bottom'
    value = 0
  [../]
  [./free]
    type = FluidFreeSurfaceBC
    variable = p
    boundary = 'front'
    alpha = '0.1'
  []
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'p'
    function = initial_cond
    boundary = 'front'
  [../]
[]

[Functions]
  [./initial_cond]
    type = ParsedFunction
    expression = '0.00001*exp(-((x-0.5)/0.1)^2-((y-0.5)/0.1)^2)'
  [../]
[]

[Materials]
  [./co_sq]
    type = GenericConstantMaterial
    prop_names = inv_co_sq
    prop_values = 4.44e-7
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  start_time = 0.0
  end_time = 0.2
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
  [./W1]
    type = PointValue
    point = '0.5 0.5 0.01'
    variable = Wave1
  [../]
[]

[Outputs]
  csv = true
  perf_graph = true
  print_linear_residuals = true
[]
