[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  ymin = 0
  xmax = 3
  ymax = 3
  nx = 3
  ny = 3
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./x_func]
    type = ParsedFunction
    expression = x
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = v
  [../]
[]

[AuxKernels]
  [./x_func_aux]
    type = FunctionAux
    variable = x
    function = x_func
    execute_on = initial
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
