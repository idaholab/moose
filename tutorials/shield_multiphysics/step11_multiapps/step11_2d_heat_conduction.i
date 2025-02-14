power = '${fparse 5e4 / 144 * 0.1}'

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step10_finite_volume/mesh2d_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete_hd concrete Al'
  []
[]

[Kernels]
  [diffusion_concrete]
    type = ADHeatConduction
    variable = T
  []
[]

[Materials]
  [concrete_hd]
    type = ADHeatConductionMaterial
    block = concrete_hd
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '5.0 + 0.001 * t'
  []
  [concrete]
    type = ADHeatConductionMaterial
    block = concrete
    temp = 'T'
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
  []
  [Al]
    type = ADHeatConductionMaterial
    block = Al
    temp = T
    thermal_conductivity_temperature_function = '175'
  []
[]

[BCs]
  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity_solid
    # 5 MW reactor, only 50 kW removed from radiation, 144 m2 cavity area
    value = '${power}'
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
    T_infinity_functor = T_fluid
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient_functor = 600
  []
[]

[Problem]
  # No kernels on the water domain
  kernel_coverage_check = false
  # No materials defined on the water domain
  material_coverage_check = false
[]

[Executioner]
  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'

  # For steady-state fixed-point iteration
  # type = Transient
  # fixed_point_max_its = 20
  # accept_on_max_fixed_point_iteration = true

  # For pseudo-transient
  type = Transient
  steady_state_detection = true
  steady_state_tolerance = 1e-6
  normalize_solution_diff_norm_by_dt = false
  start_time = -1
  dtmax = '${units 12 h -> s}'
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []
[]

[MultiApps]
  [fluid]
    # For steady-state fixed-point iteration
    # type = FullSolveMultiApp

    # For pseudo-transient
    type = TransientMultiApp

    input_files = step11_2d_fluid.i
    execute_on = 'TIMESTEP_END'
    cli_args = 'power=${power}'
  []
[]

[Transfers]
  # transfers solid temperature to nearest node on fluid mesh
  [send_T_solid]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = fluid
    source_variable = T
    variable = T_solid
    # Setting to true will cause an irrelevant warning in regions not on the boundary
    search_value_conflicts = false
  []

  # Receive fluid temperature
  [recv_T_fluid]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = fluid
    source_variable = T_fluid
    variable = T_fluid
    # Setting to true will cause an irrelevant warning in regions not on the boundary
    search_value_conflicts = false
  []
[]

[AuxVariables]
  [T_fluid]
    # This FE family-order is consistent with finite volume
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 300
  []
[]

[Postprocessors]
  [T_solid_interface_avg]
    type = SideAverageValue
    boundary = water_boundary_inwards
    variable = T
  []
[]

[Outputs]
  exodus = true
[]
