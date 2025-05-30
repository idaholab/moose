[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  [Partitioner]
    type = GridPartitioner
    nx = 2
    ny = 2
    nz = 1
  []
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

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
