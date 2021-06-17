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

[RayBCs]
  [kill1]
    type = KillRayBC
    boundary = 'top right bottom left'
    study = test1
  []
  [kill2]
    type = KillRayBC
    boundary = 'top right bottom left'
    study = test2
  []
  [kill3]
    type = KillRayBC
    boundary = 'top right bottom left'
    study = test3
  []
  [kill4]
    type = KillRayBC
    boundary = 'top right bottom left'
    study = test4
  []
[]

[RayKernels]
  [data1]
    type = TestRayDataRayKernel
    study = test1
  []
  [data2]
    type = TestRayDataRayKernel
    study = test2
  []
  [data3]
    type = TestRayDataRayKernel
    study = test3
  []
  [data4]
    type = TestRayDataRayKernel
    study = test4
  []
[]

[UserObjects]
  [test1]
    type = TestRayDataStudy
    execute_on = timestep_end

    vertex_to_vertex = true
    centroid_to_vertex = true
    centroid_to_centroid = true

    data_size = 1
    aux_data_size = 2
  []
  [test2]
    type = TestRayDataStudy
    execute_on = timestep_end

    vertex_to_vertex = true
    centroid_to_vertex = true
    centroid_to_centroid = true

    data_size = 2
    aux_data_size = 3
  []
  [test3]
    type = TestRayDataStudy
    execute_on = timestep_end

    vertex_to_vertex = true
    centroid_to_vertex = true
    centroid_to_centroid = true

    data_size = 3
    aux_data_size = 4
  []
  [test4]
    type = TestRayDataStudy
    execute_on = timestep_end

    vertex_to_vertex = true
    centroid_to_vertex = true
    centroid_to_centroid = true

    data_size = 4
    aux_data_size = 6
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Problem]
  solve = false
[]

[Adaptivity]
  steps = 1
  marker = marker
  [Markers/marker]
    type = BoxMarker
    bottom_left = '3 0 0'
    top_right = '5 5 0'
    inside = REFINE
    outside = DO_NOTHING
  []
[]

[Postprocessors]
  [ray_distance1]
    type = RayTracingStudyResult
    result = total_distance
    study = test1
  []
  [ray_distance2]
    type = RayTracingStudyResult
    result = total_distance
    study = test2
  []
  [ray_distance3]
    type = RayTracingStudyResult
    result = total_distance
    study = test3
  []
  [ray_distance4]
    type = RayTracingStudyResult
    result = total_distance
    study = test4
  []
[]

[Outputs]
  csv = true
[]
