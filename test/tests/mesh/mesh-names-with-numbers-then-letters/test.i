[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [funny_sub_name]
    input = gen
    type = RenameBlockGenerator
    old_block = "0"
    new_block = "0test"
  []
  [funny_bndry_name]
    input = funny_sub_name
    type = RenameBoundaryGenerator
    old_boundary = "right"
    new_boundary = "0test"
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = "0test"
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = "0test"
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
