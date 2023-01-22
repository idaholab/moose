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

[Functions]
  [./initial_func]
    type = ParsedFunction
    expression = sin(pi*x)*sin(pi*y)
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
  [./source]
    type = BodyForce
    variable = u
    value = 1
  [../]
[]

[BCs]
  active = 'func_bc'
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
  [./func_bc]
    type = FunctionDirichletBC
    variable = u
    boundary = 'bottom right top left'
    function = initial_func
  [../]
[]

[Postprocessors]
  [./test_pp]
    type = TestCopyInitialSolution
    execute_on = timestep_begin
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[ICs]
  [./initial]
    function = initial_func
    variable = u
    type = FunctionIC
  [../]
[]
