[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = -1
  ymin = -1
  zmax = 0
  elem_type = QUAD4
[]

[Functions]
  [./center_bubble1_x]
    type = ParsedFunction
    value = cos(t)
  [../]
  [./center_bubble1_y]
    type = ParsedFunction
    value = sin(t)
  [../]
  [./center_bubble2_x]
    type = ParsedFunction
    value = 0
  [../]
  [./center_bubble2_y]
    type = ParsedFunction
    value = cos(t)
  [../]
  [./center_bubble3_x]
    type = ParsedFunction
    value = cos(t+pi/2)
  [../]
  [./center_bubble3_y]
    type = ParsedFunction
    value = 0
  [../]
[]

[Variables]
  [./u0]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./grain_map]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u0
    coef = 20
  [../]
  [./forcing1]
    type = SharpInterfaceForcing
    variable = u0
    x_center = center_bubble1_x
    y_center = center_bubble1_y
    amplitude = 1
  [../]
  [./forcing2]
    type = SharpInterfaceForcing
    variable = u0
    x_center = center_bubble2_x
    y_center = center_bubble2_y
    amplitude = 1
  [../]
  [./forcing3]
    type = SharpInterfaceForcing
    variable = u0
    x_center = center_bubble3_x
    y_center = center_bubble3_y
    amplitude = 1
  [../]
  [./dot]
    type = TimeDerivative
    variable = u0
  [../]
[]

[AuxKernels]
  active = ''
  [./mapper]
    # type = NodalFloodCountAux
    type = GrainTrackerAux
    variable = grain_map
    execute_on = timestep
    bubble_object = grains
  [../]
[]

[BCs]
  active = 'Periodic'
  [./all]
    type = DirichletBC
    value = 0
    variable = u0
    boundary = 'left right top bottom'
  [../]
  [./Periodic]
    [./all_pbc]
      auto_direction = 'x y'
    [../]
  [../]
[]

[UserObjects]
  active = ''
  [./grains]
    # type = NodalFloodCount 
    type = GrainTracker
    threshold = 0.2
    execute_on = timestep
    crys_num = 1
    var_name_base = u
    tracking_step = 2
    variable = u0
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'
  num_steps = 400
  dt = 0.1
[]

[Output]
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

