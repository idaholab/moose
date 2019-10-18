[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = reactor.e
  []

  [./generate_sidesets]
    type = AllSideSetsByNormalsGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
