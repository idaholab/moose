[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete concrete_hd Al'
  []
[]

[Kernels]
  [diffusion_concrete_hd]
    type = CoefDiffusion
    variable = T
    coef = 5
    block = concrete_hd
  []
  [diffusion_concrete]
    type = CoefDiffusion
    variable = T
    coef = 2.25
    block = concrete
  []
  [diffusion_Al]
    type = CoefDiffusion
    variable = T
    coef = 175
    block = Al
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity_solid
    # 5 MW reactor, only 50kW removed through radiation, 144 m2 cavity area
    value = '${fparse 5e4 / 144}'
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
    heat_transfer_coefficient = 600
  []
[]

[Problem]
  # No kernels on the water domain
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady # Steady state problem
  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true # Output Exodus format
[]
