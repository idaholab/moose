[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  partitioner = linear
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./someaux]
  [../]
  [./otheraux]
    family = MONOMIAL
    order = CONSTANT
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
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [./nl_wb]
    type = WorkBalance
    execute_on = initial
    system = nl
  []

  [./aux_wb]
    type = WorkBalance
    execute_on = initial
    system = aux
  []

  [./all_wb]
    type = WorkBalance
    execute_on = initial
    system = all
  []
[]

[Outputs]
  csv = true
[]
