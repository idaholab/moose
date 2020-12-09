[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    nz = 5
    xmax = 5
    ymax = 5
    zmax = 5
  []
[]

[RayBCs]
  active = 'kill_2d'
  [kill_1d]
    type = KillRayBC
    boundary = 'left right'
  []
  [kill_2d]
    type = KillRayBC
    boundary = 'top right bottom left'
  []
  [kill_3d]
    type = KillRayBC
    boundary = 'top right bottom left front back'
  []
[]

# Add a dummy RayKernel to enable additional error
# checking before onSegment() is called
[RayKernels/null]
  type = NullRayKernel
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  vertex_to_vertex = false
  centroid_to_vertex = false
  centroid_to_centroid = false
  side_aq = false
  centroid_aq = false
  compute_expected_distance = true
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
