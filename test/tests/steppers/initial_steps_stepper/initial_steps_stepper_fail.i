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
  fail_step = 3
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 10
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [./Steppers]
    [./simple]
      type = SimpleStepper
      dt = 2
      growth_factor = 1.5
    [../]
    [./initial]
      type = InitialStepsStepper
      incoming_stepper = simple
      dt = 0.5
      n_steps = 4
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
