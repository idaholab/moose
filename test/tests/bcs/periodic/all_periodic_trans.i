[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmax = 10
  ymax = 10
  zmin = 0
  zmax = 0
  elem_type = QUAD4
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in PeriodicBCs.
  distribution = serial
[]

[Functions]
  [./tr_x]
    type = ParsedFunction
    value = x
  [../]
  [./tr_y]
    type = ParsedFunction
    value = y+10
  [../]
  [./itr_x]
    type = ParsedFunction
    value = x
  [../]
  [./itr_y]
    type = ParsedFunction
    value = y-10
  [../]

  [./tr_x2]
    type = ParsedFunction
    value = x+10
  [../]
  [./tr_y2]
    type = ParsedFunction
    value = y
  [../]
  [./itr_x2]
    type = ParsedFunction
    value = x-10
  [../]
  [./itr_y2]
    type = ParsedFunction
    value = y
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./forcing]
    type = GaussContForcing
    variable = u
    x_center = 2
    y_center = 1
    x_spread = 0.25
    y_spread = 0.5
  [../]
  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  # active = ' '
  [./Periodic]
    [./x]
      primary = bottom
      secondary = top
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'itr_x itr_y'
    [../]

    [./y]
      primary = left
      secondary = right
      transform_func = 'tr_x2 tr_y2'
      inv_transform_func = 'itr_x2 itr_y2'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 10
  solve_type = NEWTON
[]

[Output]
  interval = 1
  exodus = true
  perf_log = true
[]

