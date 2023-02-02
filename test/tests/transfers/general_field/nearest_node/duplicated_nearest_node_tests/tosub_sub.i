[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD8
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[AuxVariables]
  [./nodal_source_from_parent_nodal]
    family = LAGRANGE
    order = FIRST
  [../]
  [./nodal_source_from_parent_elemental]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./elemental_source_from_parent_nodal]
    family = LAGRANGE
    order = FIRST
  [../]
  [./elemental_source_from_parent_elemental]
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
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
