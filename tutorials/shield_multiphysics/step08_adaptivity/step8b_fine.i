[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
  []

  uniform_refine = 2
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
[]

[Outputs]
  exodus = true # Output Exodus format
[]

[Postprocessors]
  [num_elements]
    type = NumElems
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
