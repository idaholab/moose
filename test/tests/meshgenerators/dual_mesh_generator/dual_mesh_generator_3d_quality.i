[Mesh]
  [primal]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX8
  []

  [dual]
    type = DualMeshGenerator
    input = primal
    dual_mesh_type = barycentric
    concave_treatment = 'split polycut netgen'
  []
[]

[UserObjects]
  [edge_length_ratio]
    type = ElementQualityChecker
    metric_type = EDGE_LENGTH_RATIO
    lower_bound = 1e-12
    upper_bound = 1e6
    failure_type = WARNING
  []
  [jacobian]
    type = ElementQualityChecker
    metric_type = JACOBIAN
    lower_bound = 1e-12
    upper_bound = 1e6
    failure_type = WARNING
  []
  [scaled_jacobian]
    type = ElementQualityChecker
    metric_type = SCALED_JACOBIAN
    lower_bound = 1e-12
    upper_bound = 1.0000001
    failure_type = WARNING
  []
  [minimum_angle]
    type = ElementQualityChecker
    metric_type = MIN_ANGLE
    lower_bound = 1e-8
    upper_bound = 180.0000001
    failure_type = WARNING
  []
  [maximum_angle]
    type = ElementQualityChecker
    metric_type = MAX_ANGLE
    lower_bound = 1e-8
    upper_bound = 180.0000001
    failure_type = WARNING
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
