[Mesh]
  [file]
    type = FileMeshGenerator
    file = nonplanar.e
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top right bottom left front back'
[]

[RayKernels/null]
  type = NullRayKernel
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = true
  side_aq = true
  centroid_aq = true
  compute_expected_distance = true
  warn_non_planar = false
  execute_on = initial
[]

[Postprocessors]
  [total_distance]
    type = RayTracingStudyResult
    study = lots
    result = total_distance
  []
  [expected_distance]
    type = LotsOfRaysExpectedDistance
    lots_of_rays_study = lots
  []
  [distance_difference]
    type = DifferencePostprocessor
    value1 = total_distance
    value2 = expected_distance
  []
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
