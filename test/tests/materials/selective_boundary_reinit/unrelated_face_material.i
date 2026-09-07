[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
[]

[AuxVariables]
  [u]
  []
[]

[Materials]
  [volume]
    type = BoundaryMaterialReinitTest
    property = volume_property
    value = 2
    error_on_face = true
  []
[]

[Postprocessors]
  [volume_average]
    type = ElementAverageMaterialProperty
    mat_prop = volume_property
    execute_on = TIMESTEP_END
  []
  [side_average]
    type = SideAverageValue
    variable = u
    boundary = left
    execute_on = TIMESTEP_END
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
[]
