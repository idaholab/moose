[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    ymax = 0.1
    xmax = 0.1
  []

  uniform_refine = 0
[]

[Adaptivity]
  max_h_level = 4
  initial_steps = 6
  initial_marker = error_marker
  cycles_per_step = 2
  marker = error_marker
  [Indicators]
    [phi_jump]
      type = GradientJumpIndicator
      variable = phi
    []
  []
  [Markers]
    [error_marker]
      type = ErrorFractionMarker
      indicator = phi_jump
      refine = 0.8
      coarsen = 0.1
    []
  []
[]

[Variables]
  [temperature]
    initial_condition = 300
  []
[]

[AuxVariables]
  [phi]
  []
  [por_var]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [corrosion]
    type = RandomCorrosion
    variable = phi
    reference_temperature = 300
    temperature = temperature_in
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [por_var]
    type = ADMaterialRealAux
    variable = por_var
    property = porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[BCs]
  [left]
    type = PostprocessorDirichletBC
    variable = temperature
    boundary = left
    postprocessor = temperature_in
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 100 # prescribed flux
  []
[]

[Materials]
  [column]
    type = PackedColumn
    temperature = temperature
    radius = 1 # mm
    phase = phi
  []
[]

[Postprocessors]
  [temperature_in]
    type = Receiver
    default = 301
  []
  [k_eff]
    type = ThermalConductivity
    variable = temperature
    T_hot = temperature_in
    flux = 100
    dx = 0.1
    boundary = right
    length_scale = 1
    k0 = 12.05
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [por_var]
    type = ElementAverageValue
    variable = por_var
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [t_right]
    type = SideAverageValue
    boundary = right
    variable = temperature
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  end_time = 1000
  dt = 1
  steady_state_tolerance = 1e-9
  steady_state_detection = true
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  automatic_scaling = true
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
[]

[ICs]
  [close_pack]
    radius = 0.01 # meter
    outvalue = 0  # water
    variable = phi
    invalue = 1   # steel
    type = ClosePackIC
  []
[]
