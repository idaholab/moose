[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    nx = 5
    ny = 5
    nz = 5
    xmax = 5
    ymax = 5
    zmax = 5
  []
[]

[RayBCs]
  active = ''
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

[UserObjects/study]
  type = BackfaceCullingStudyTest

  ray_kernel_coverage_check = false
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = true
  side_aq = true
  centroid_aq = true
  edge_to_edge = false
  compute_expected_distance = true

  execute_on = initial
[]

[Postprocessors]
  [total_distance]
    type = RayTracingStudyResult
    study = study
    result = total_distance
  []
  [expected_distance]
    type = LotsOfRaysExpectedDistance
    lots_of_rays_study = study
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
