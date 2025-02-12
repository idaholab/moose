[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../step03_boundary_conditions/mesh_in.e'
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

[DiracKernels]
  [detector_heat]
    type = ReporterPointSource
    variable = T
    block = Al
    point_name = 'detector_positions/positions_1d'
    value_name = 'receiver/detector_heat'
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
  # No materials defined on the water domain
  material_coverage_check = false
[]

[Executioner]
  type = Steady # Steady state problem

  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'

  # Fixed point iteration parameters
  fixed_point_max_its = 10
[]

[Positions]
  [detector_positions]
    type = FilePositions
    files = detector_positions.txt
  []
[]

[MultiApps]
  [detectors]
    type = FullSolveMultiApp
    input_files = 'step11_local.i'
    # Create one app at each position
    positions_objects = 'detector_positions'

    # displace the subapp output to their position in the parent app frame
    output_in_position = true

    # compute the global temperature first
    execute_on = 'TIMESTEP_END'
  []
[]

[Transfers]
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
  [detector_heat]
    type = MultiAppReporterTransfer
    from_multi_app = detectors
    from_reporters = 'heat_flux/value'
    to_reporters = 'receiver/detector_heat'
    distribute_reporter_vector = true
  []
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

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = 'detector_heat'
    real_vector_values = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
  []
[]

reactor_x = 3.275
reactor_y = 4.625
reactor_z = 2.225
[AuxVariables]
  [flux]
    [InitialCondition]
      type = FunctionIC
      function = '4e6 * exp(-((x-${reactor_x})^2 + (y-${reactor_y})^2 + (z-${reactor_z})^2))'
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
