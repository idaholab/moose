[Mesh]
  # a dummy mesh
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 1
  ny = 1
  elem_type = QUAD4
[]

[Variables]
  [./n]
    family = SCALAR
    order = FIRST
  [../]
[]

[Functions]
  [./f]
    type = ParsedFunction
    expression = cos(t)
  [../]
[]

[ICs]
  [./f]
    type = FunctionScalarIC
    variable = n
    function = f
  [../]
[]

[ScalarKernels]
  [./dn]
    type = ODETimeDerivative
    variable = n
  [../]
  [./ode1]
    type = ParsedODEKernel
    expression = '-n'
    variable = n
  [../]
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 1
  dt = 0.01
  scheme = bdf2
  solve_type = 'PJFNK'
  timestep_tolerance = 1e-12
[]

[Outputs]
  csv = true
[]
