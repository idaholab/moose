[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./generate_sidesets]
    type = SideSetsFromAllNormalsGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
