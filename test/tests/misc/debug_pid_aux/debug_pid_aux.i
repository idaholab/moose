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

[Debug]
  pid_aux = true
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
