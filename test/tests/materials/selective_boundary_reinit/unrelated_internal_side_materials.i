[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
[]

[AuxVariables]
  [u]
  []
[]

[Materials]
  [volume]
    type = BoundaryMaterialReinitTest
    property = volume_property
    error_on_face = true
    error_on_neighbor = true
  []
[]

[UserObjects]
  [internal_side]
    type = InsideUserObject
    variable = u
    execute_on = TIMESTEP_END
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1
[]
