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
    extra_matrix_tags = 'time'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    extra_matrix_tags = 'time'
    extra_vector_tags = 'nontime'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    extra_matrix_tags = 'time'
    extra_vector_tags = 'nontime'
    value = 1
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 20000
  dt = 0.001
  solve_type = LINEAR
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  steady_state_detection = true
  [./TimeIntegrator]
    type = LumpedExplicitEuler
  [../]
[]

[Outputs]
  exodus = true
[]