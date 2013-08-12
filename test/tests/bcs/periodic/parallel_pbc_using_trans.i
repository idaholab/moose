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
  # due to a bug in PeriodicBCs.  Note: it might work with 2 processors
  # but not with 4.
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
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 10
[]

[Output]
  interval = 1
  exodus = true
  perf_log = true
[]

