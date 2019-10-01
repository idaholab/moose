[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = twoblocks.e
  []

  [./generate_sidesets]
    type = AllSideSetsByNormalsGenerator
    input = fmg
  []
[]

[Outputs]
  exodus = true
[]
