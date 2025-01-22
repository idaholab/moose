[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete concrete_and_Al'
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
  exodus = true
  csv = true
[]

[Postprocessors]
  [num_elements]
    type = NumElems
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [average_temperature]
    type = ElementAverageValue
    variable = T
    block = 'concrete'
  []
  [water_heat_flux]
    type = ADSideDiffusiveFluxIntegral
    variable = T
    boundary = water_boundary_inwards
    diffusivity = 2.25
  []
[]

[VectorPostprocessors]
  [temperature_sample]
    type = LineValueSampler
    num_points = 500
    start_point = '0.1 0      0'
    end_point = '0.1 10 0'
    variable = T
    sort_by = y
  []
[]
