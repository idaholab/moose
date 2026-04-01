[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = reactor.e
  []

  [./generate_sidesets]
    type = SideSetsFromAllNormalsGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
