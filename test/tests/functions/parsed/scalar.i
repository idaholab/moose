[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./scalar]
    family = SCALAR
    initial_condition = 0
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[AuxScalarKernels]
  [./scalar_aux]
    type = FunctionScalarAux
    variable = scalar
    function = func
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = left_bc
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Functions]
  [./left_bc]
    type = ParsedFunction
    expression = s
    symbol_values = scalar
    symbol_names = s
  [../]
  [./func]
    type = ParsedFunction
    expression = t
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

