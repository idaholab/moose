[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  [Partitioner]
    type = HierarchicalGridPartitioner
    nx_nodes = 2
    ny_nodes = 2
    nx_procs = 2
    ny_procs = 2
  []
[]

[Variables/u]
[]

[Kernels/diff]
  type = Diffusion
  variable = u
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
[]

[Outputs]
  exodus = true
[]

[AuxVariables/pid]
  family = MONOMIAL
  order = CONSTANT
[]

[Problem]
  solve = false
[]

[AuxKernels/pid]
  type = ProcessorIDAux
  variable = pid
  execute_on = 'INITIAL'
[]
