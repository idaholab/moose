[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  [Partitioner]
    type = GridPartitioner
    nx = 2
    ny = 2
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

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
  [algebraic1]
    family = MONOMIAL
    order = CONSTANT
  []
  [geometric1]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pid]
    type = ProcessorIDAux
    variable = pid
  []
  [geometric1]
    type = GhostingAux
    pid = 1
    variable = geometric1
    functor_type = geometric
  []
  [algebraic1]
    type = GhostingAux
    functor_type = algebraic
    pid = 1
    variable = algebraic1
  []
[]

[Problem]
  default_ghosting = true
[]
