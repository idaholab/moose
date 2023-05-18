[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
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
    boundary = left
    value = 0
  []
  [right]
    type = PostprocessorDirichletBC
    variable = u
    boundary = right
    postprocessor = from_parent
  []
[]

[Postprocessors]
  [from_parent]
    type = Receiver
  []
  [to_parent]
    type = PointValue
    variable = u
    point = '0.5 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]
