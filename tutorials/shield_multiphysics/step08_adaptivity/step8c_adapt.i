[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
[]

[Adaptivity]
  marker = jump_threshold
  max_h_level = 2
  [Indicators]
    [temperature_jump]
      type = GradientJumpIndicator
      variable = T
      scale_by_flux_faces = true
    []
  []
  [Markers]
    [jump_threshold]
      type = ValueThresholdMarker
      coarsen = 0.3
      variable = temperature_jump
      refine = 4
      block = 'concrete'
    []
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
  []
[]

[Kernels]
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
  []
  [time_derivative]
    type = TimeDerivative
    variable = T
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    value = '${fparse 1e5 / 108}'
  []
  [air_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'air_boundary'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 10
  []
  [ground]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'ground'
  []
  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 30
  []
[]

[Problem]
  type = FEProblem
  # No kernels on the water domain
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true # Output Exodus format
[]

[Postprocessors]
  [num_elements]
    type = NumElements
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
