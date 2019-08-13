[Mesh]
  [deleter]
    type = PlaneDeletionGenerator
    point = '0.5 0.5 0'
    normal = '-1 0 0'
    input = generated
    new_boundary = 6
  []
  [generated]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
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
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
