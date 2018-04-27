[GlobalParams]
  implicit = false
[]

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
    implicit = true
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 10
  dt = 0.001
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  [../]
[]

[Outputs]
  exodus = true
[]