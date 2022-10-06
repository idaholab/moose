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
  output_process_domains = true
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
