[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 1
  elem_type = QUAD4
[]

[UserObjects]
  [element_quality]
    type = ElementQualityChecker
    metric_type = MIN_ANGLE
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
