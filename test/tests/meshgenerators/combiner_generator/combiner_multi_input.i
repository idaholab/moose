[Mesh]
  [gen1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin=0
    xmax=1
    ymin=0
    ymax=1
  []
  [gen2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 12
    ny = 12
    xmin=2
    xmax=3
    ymin=2
    ymax=3
  []
  [gen3]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 14
    ny = 14
    xmin=3.5
    xmax=5
    ymin=3
    ymax=4
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'gen1 gen2 gen3'
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
