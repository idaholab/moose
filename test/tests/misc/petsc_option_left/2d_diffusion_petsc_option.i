[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'LINEAR'

  start_time = 0.0
  num_steps = 1
  dt = 0.00005

  [./TimeIntegrator]
    type = ActuallyExplicitEuler
  [../]

  petsc_options_iname = "-pc_type"
  petsc_options_value = "hypre"
[]
