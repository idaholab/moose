[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

[Variables/phase]
  [InitialCondition]
    type = FunctionIC
    variable = field
    function = '(x > 2.99) * 1.0'
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top right bottom left'
[]

[RayKernels/test]
  type = RefractionRayKernelTest
  field = phase
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = false
  execute_on = initial
[]

[Postprocessors/total_distance]
  type = RayTracingStudyResult
  study = lots
  result = total_distance
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = false
  csv = true
[]
