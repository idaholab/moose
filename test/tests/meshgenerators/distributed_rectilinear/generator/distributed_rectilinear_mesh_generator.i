[Mesh]
  [gmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 100
    ny = 100
  []
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
  [npid]
    family = Lagrange
    order = first
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
  [npid_aux]
    type = ProcessorIDAux
    variable = npid
    execute_on = 'INITIAL'
  []
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
    preset = false
    boundary = 'left'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
