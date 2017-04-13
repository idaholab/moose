[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = -2
  xmax = 2
  ymin = -2
  ymax = 2
  zmax = 0
  elem_type = QUAD4
[]

[Functions]
  active = 'center_bubble2_y center_bubble2_x center_bubble1_x center_bubble1_y'
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
    value = cos(t+pi)
  [../]
  [./center_bubble2_y]
    type = ParsedFunction
    value = sin(t+pi)
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
  active = 'diff dot forcing1 forcing2'
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
  [../]
  [./forcing2]
    type = SharpInterfaceForcing
    variable = u0
    x_center = center_bubble2_x
    y_center = center_bubble2_y
  [../]
  [./forcing3]
    type = SharpInterfaceForcing
    variable = u0
    x_center = center_bubble3_x
    y_center = center_bubble3_y
    amplitude = -1
  [../]
  [./dot]
    type = TimeDerivative
    variable = u0
  [../]
[]

[AuxKernels]
  [./mapper]
    type = FeatureFloodCountAux
    variable = grain_map
    execute_on = 'initial timestep_end'
    flood_counter = grains
    field_display = UNIQUE_REGION
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    value = 0
    variable = u0
    boundary = 'left right top bottom'
  [../]
[]

[Postprocessors]
  [./grains]
    type = GrainTracker
    threshold = 0.0005
    op_num = 1
    var_name_base = u
    tracking_step = 1
    variable = u0
    remap_grains = false
    flood_entity_type = NODAL
    execute_on = 'initial timestep_end'
  [../]
  [./elem_avg_value]
    type = ElementAverageValue
    variable = u0
    execute_on = linear
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  solve_type = PJFNK
  num_steps = 400
  dt = 0.1
[]

[Outputs]
#  output_initial = true
#  interval = 1
  exodus = true
#  perf_log = true
[]
