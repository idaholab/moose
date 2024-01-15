[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'mesh_coarse_in.e'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete concrete_inner'
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
    block = 'concrete concrete_inner'
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
    # Power is reduced as cooling was further removed
    # 10 kW reactor, 108 m2 cavity area
    value = '${fparse 1e4 / 108}'
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
[]

[Executioner]
  type = Steady # Steady state problem
  solve_type = NEWTON # Perform a Newton solve, uses AD to compute Jacobian terms
  petsc_options_iname = '-pc_type -pc_hypre_type' # PETSc option pairs with values below
  petsc_options_value = 'hypre boomeramg'
[]

[Positions]
  [all_elements]
    type = ElementCentroidPositions
    block = concrete_inner
  []
[]

[MultiApps]
  [detectors]
    type = FullSolveMultiApp
    input_files = 'step11_local.i'
    # Create one app at each position
    positions_objects = 'all_elements'

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
  [flux]
    [InitialCondition]
      type = FunctionIC
      function = '10 * exp(-((x-5)*(x-5) + (y-7) * (y-7) + (z - 3)*(z-3)))'
    []
  []

  # We only output two fields as an example
  [T_hdpe_inner]
    family = MONOMIAL
    order = CONSTANT
    block = concrete_inner
  []
  [T_boron_inner]
    family = MONOMIAL
    order = CONSTANT
    block = concrete_inner
  []
[]

[Outputs]
  exodus = true
[]
