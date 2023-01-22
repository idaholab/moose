# All tested logic is in the aux system
# The non-linear problem is unrelated

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -1
  xmax =  1
  nx = 10
[]

[Functions]
  [./v1_func]
    type = ParsedFunction
    expression = (1-x)/2
  [../]
  [./v2_func]
    type = ParsedFunction
    expression = (1+x)/2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./lc]
  [../]

  [./v1]
  [../]
  [./v2]
  [../]

  [./w1]
  [../]
  [./w2]
  [../]
[]

[ICs]
  [./v1_ic]
    type = FunctionIC
    variable = v1
    function = v1_func
  [../]
  [./v2_ic]
    type = FunctionIC
    variable = v2
    function = v2_func
  [../]

  [./w1_ic]
    type = ConstantIC
    variable = w1
    value = 0.3
  [../]
  [./w2_ic]
    type = ConstantIC
    variable = w2
    value = 0.5
  [../]
[]

[AuxKernels]
  [./lc-aux]
    type = ParsedAux
    variable = lc
    expression = 'v1*w1+v2*w2'
    coupled_variables = 'v1 w1 v2 w2'
    execute_on = 'timestep_end'
  [../]
[]


[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 2
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]
