[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = poly2.msh
  []
[]

[Outputs]
  exodus = true
[]

# to replicate run this with --mesh-only --distributed-mesh
