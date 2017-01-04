[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  ymax = 0.1
  xmax = 0.1
  uniform_refine = 0
[]

[Adaptivity]
  max_h_level = 4
  initial_steps = 6
  cycles_per_step = 2
  initial_marker = error_marker
  marker = error_marker
  [./Indicators]
    [./phi_jump]
      type = GradientJumpIndicator
      variable = phi
    [../]
  [../]
  [./Markers]
    [./error_marker]
      type = ErrorFractionMarker
      indicator = phi_jump
      refine = 0.9
    [../]
  [../]
[]

[Variables]
  [./temp]
    initial_condition = 300
  [../]
[]

[AuxVariables]
  [./phi]
  [../]
[]

[AuxKernels]
  [./corrosion]
    type = RandomCorrosion
    execute_on = 'timestep_end'
    variable = phi
    reference_temperature = 300
    temperature = temp_in
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./left]
    type = PostprocessorDirichletBC
    variable = temp
    boundary = left
    postprocessor = temp_in
  [../]
  [./right]
    type = NeumannBC
    variable = temp
    boundary = right
    value = 100 # prescribed flux

  [../]
[]

[Materials]
  [./column]
    type = PackedColumn
    block = 0
    sphere_radius = 1 # mm
    phase = phi
  [../]
[]

[Problem]
  type = FEProblem
[]

[Postprocessors]
  [./temp_in]
    type = Receiver
    default = 301
  [../]
  [./k_eff]
    type = ThermalConductivity
    variable = temp
    T_hot = temp_in
    flux = 100
    dx = 0.1
    boundary = right
    length_scale = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 50
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  execute_on = 'initial timestep_end'
  exodus = true
[]

[ICs]
  [./close_pack]
    radius = 0.01
    outvalue = 0 # water
    variable = phi
    invalue = 1 #steel
    type = ClosePackIC
  [../]
[]
