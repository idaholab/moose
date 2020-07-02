[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 5
  ny = 5
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Controls]
  [doff]
    type = TimePeriod
    enable_objects = 'DiracKernel::point_source'
    disable_objects = 'DiracKernel::point_source2'
    start_time = 0
    end_time = 2
  []
[]

[DiracKernels]
  [./point_source]
    type = FunctionDiracSource
    variable = u
    function = 1
    point = '0.3 0.3 0.0'
  [../]
  [./point_source2]
    type = FunctionDiracSource
    variable = u
    function = 1
    point = '-0.3 -0.3 0.0'
  [../]
[]

[BCs]
  [./external]
    type = NeumannBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  l_tol = 1e-03
[]

[Outputs]
  exodus = true
[]
