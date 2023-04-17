[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    xmin = -3
    xmax = 0
    ymin = -5
    ymax = 5
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    xmin = 3
    xmax = 6
    ymin = -5
    ymax = 5
  []

  [left_and_right]
    type = MeshCollectionGenerator
    inputs = 'left right'
  []

  [omg]
    type = OverlayMeshGenerator
    input = 'left_and_right'
    dim = 2
    nx = 2
    ny = 2
    xmin = -3
    xmax = 6
    ymin = -5.5
    ymax = 5.5
    save_with_name='overlay_mesh'
    output = true
  []
  final_generator = 'left_and_right'
[]


[Problem]
  solve = false
[]

[UserObjects/test]
  type = RayTracingOverlayMeshTest
  overlay_mesh = 'overlay_mesh'
[]

[Reporters/find_intersection_elems]
  type = IntersectionElemsReporter
  overlay_uo_name = test
  execute_on = initial
[]

[Executioner]
  type = Steady
[]


[Outputs]
  file_base = 'find_intersection_elems'
  [out]
    type = JSON
    execute_on = 'FINAL'
    execute_system_information_on = none
  []
[]
