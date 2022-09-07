[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Postprocessors]
  [pp]
    type = ElementAverageMaterialProperty
    mat_prop = blah
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
