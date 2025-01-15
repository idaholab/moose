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
  max_h_level = 3
  initial_steps = 5
  cycles_per_step = 2
  initial_marker = error_marker
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
      refine = 0.9
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
[]

[AuxKernels]
  [corrosion]
    type = RandomCorrosion
    execute_on = 'timestep_end'
    variable = phi
    reference_temperature = 300
    temperature = 301
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
    postprocessor = 301
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
    outputs = exodus
    output_properties = porosity
  []
[]

[Problem]
  type = FEProblem
[]

[Postprocessors]
  [k_eff]
    type = ThermalConductivity
    variable = temperature
    T_hot = 301
    flux = 100
    dx = 0.1
    boundary = right
    length_scale = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
  [console]
    type = Console
    execute_postprocessors_on = 'timestep_begin timestep_end'
  []
[]

[ICs]
  [close_pack]
    radius = 0.01
    outvalue = 0 # water
    variable = phi
    invalue = 1 #steel
    type = ClosePackIC
  []
[]
