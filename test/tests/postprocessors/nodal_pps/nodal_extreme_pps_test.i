[Mesh]
  type = FileMesh
  file = trapezoid.e
  uniform_refine = 1
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in PeriodicBCs.
  distribution = serial
[]

[Functions]
  [./tr_x]
    type = ParsedFunction
    value = -x*cos(pi/3)
  [../]
  [./tr_y]
    type = ParsedFunction
    value = x*sin(pi/3)
  [../]
  [./itr_x]
    type = ParsedFunction
    value = -x/cos(pi/3)
  [../]
  [./itr_y]
    type = ParsedFunction
    value = 0
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
    y_center = -1
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
      primary = 1
      secondary = 4
      transform_func = 'tr_x tr_y'
      inv_transform_func = 'itr_x itr_y'
    [../]
  [../]
[]

[Postprocessors]
  [./max_nodal_pps]
    type = NodalExtremeValue
    variable = u
  [../]
  [./max_node_id]
    type = NodalProxyMaxValue
    variable = u
  [../]
  [./min_nodal_pps]
    type = NodalExtremeValue
    variable = u
    value_type = min
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.5
  num_steps = 6
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
