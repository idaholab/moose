# Real facility uses forced convection to cool the water tank at full power
# Need to lower power for natural convection so concrete doesn't get too hot.
power = '${fparse 5e4 / 144 * 0.5}'

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh2d_coarse_in.e'
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
  # No materials on the water domain
  material_coverage_check = false
[]

[Executioner]
  # For pseudo-transient
  type = Transient
  start_time = -1
  end_time = ${units 4 h -> s}
  dtmax = 100
  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0.1, 0.1, t)'
  []

  # For steady-state fixed-point iteration
  # type = Steady
  # fixed_point_max_its = 20
  # accept_on_max_fixed_point_iteration = true

  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-8
[]

[Positions]
  [detector_positions]
    type = FilePositions
    files = detector_positions_2d.txt
  []
[]

[MultiApps]
  [fluid]
    # For pseudo-transient
    type = TransientMultiApp

    # For steady-state fixed-point iteration
    # type = FullSolveMultiApp

    input_files = step11_2d_fluid.i
    execute_on = 'TIMESTEP_END'

    # Pass in parameter values as if from command line
    cli_args = 'power=${power}'
  []
  [detectors]
    type = FullSolveMultiApp
    input_files = 'step11_local.i'
    # Create one app at each position
    positions_objects = 'detector_positions'

    # displace the subapp output to their position in the parent app frame
    output_in_position = true

    # compute the global temperature first
    execute_on = 'TIMESTEP_END'

    # Pass in parameter values as if from command line
    cli_args = 'Outputs/console=false'
  []
[]

[Transfers]
  # transfers solid temperature to nearest node on fluid mesh
  [send_T_solid]
    type = MultiAppCopyTransfer
    to_multi_app = fluid
    source_variable = T
    variable = T_solid
  []
  # Receive fluid temperature
  [recv_T_fluid]
    type = MultiAppCopyTransfer
    from_multi_app = fluid
    source_variable = T_fluid
    variable = T_fluid
    to_blocks = 'water'
    from_blocks = 'water'
  []

  # transfers local boundary temperature to the each child app
  [send_exterior_temperature]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = detectors
    source_variable = T
    postprocessor = T_boundary
  []
  # transfers local flux conditions to each child app
  [send_local_flux]
    type = MultiAppVariableValueSampleTransfer
    to_multi_app = detectors
    source_variable = flux
    variable = flux
  []
  # retrieve outputs from the child apps
  [hdpe_temperature]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = detectors
    postprocessor = T_hdpe_inner
    variable = T_hdpe_inner
  []
  [boron_temperature]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = detectors
    postprocessor = T_boron_inner
    variable = T_boron_inner
  []
[]

[AuxVariables]
  [T_fluid]
    # family = MONOMIAL
    # order = CONSTANT
    type = INSFVEnergyVariable
    initial_condition = 300
    block = 'water'
  []

  [flux]
    [InitialCondition]
      type = FunctionIC
      function = '1e4 * exp(-((x-3.25)^2 + (y-2.225)^2))'
    []
  []

  # We only output two fields as an example
  [T_hdpe_inner]
    family = MONOMIAL
    order = CONSTANT
    block = Al
  []
  [T_boron_inner]
    family = MONOMIAL
    order = CONSTANT
    block = Al
  []
[]

[Outputs]
  exodus = true
[]
