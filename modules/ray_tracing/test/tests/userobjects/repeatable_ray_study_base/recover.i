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

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top right bottom left'
[]

[UserObjects/lots]
  type = TestRayDataStudy
  centroid_to_centroid = true
  vertex_to_vertex = true
  centroid_to_vertex = true
  execute_on = timestep_end
  compute_expected_distance = true
  data_size = 3
  aux_data_size = 2
[]

[RayKernels/data]
  type = TestRayDataRayKernel
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Problem]
  solve = false
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

[Outputs]
  csv = true
[]
