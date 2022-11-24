[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Functions]
  [./dts]
    type = PiecewiseLinear
    x = '0   0.85  2'
    y = '0.2 0.25  0.25'
  [../]
[]

[Variables]
  [./u]
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

[BCs]
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
[]

[Problem]
  type = FailingProblem
  fail_steps = '3'
[]

[Executioner]
  type = Transient
  num_steps = 10
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [./TimeStepper]
    type = FunctionDT
    function = dts
    min_dt = 0.01
    cutback_factor_at_failure = 0.75
  [../]
[]

[Outputs]
  exodus = true
[]
