[Mesh]
  file = gap_test.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./t]
    type = ParsedFunction
    value = t
  [../]

  [./zero]
    type = ParsedFunction
    value = 0
  [../]
[]

[AuxVariables]
  [./distance]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff u_time'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./u_time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxKernels]
  [./x]
    type = FunctionAux
    variable = disp_x
    function = zero
    block = 1
  [../]

  [./y]
    type = FunctionAux
    variable = disp_y
    function = zero
    block = 1
  [../]

  [./z]
    type = FunctionAux
    variable = disp_z
    function = t
    block = 1
  [../]
[]

[AuxBCs]
  active = 'gap_distance gap_distance2'

  [./gap_distance]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 2
    paired_boundary = 3
  [../]

  [./gap_distance2]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 3
    paired_boundary = 2
  [../]
[]

[BCs]
  active = 'block1_left block1_right block2_left block2_right'

  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  dt = 1.0
  num_steps = 8
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


