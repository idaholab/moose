[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10

  [Partitioner]
    type = SingleRankPartitioner
    rank = 2
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
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = ProcessorIDAux
      execute_on = 'TIMESTEP_END'
    []
  []
[]

[Outputs]
  exodus = true
[]
