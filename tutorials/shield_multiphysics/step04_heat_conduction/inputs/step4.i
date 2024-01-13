[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
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
    type = ADHeatConduction
    variable = T
  []
[]

[Materials]
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = '1170'
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
  material_coverage_check = false
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
