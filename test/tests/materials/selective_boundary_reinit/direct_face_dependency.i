[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Materials]
  [face_source]
    type = BoundaryMaterialReinitTest
    property = face_property
    value = 2
  []
[]

[Postprocessors]
  [face_average]
    type = SideAverageMaterialProperty
    boundary = left
    property = face_property
    execute_on = INITIAL
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
