[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[RayKernels/null]
  type = NullRayKernel
  rays = 'ray'
[]

[UserObjects/study]
  type = RepeatableRayStudy
  names = 'ray'
  start_points = '0 0 0'
  end_points = '0.99 0 0'
[]

[Postprocessors/ray_distance]
  type = RayTracingStudyResult
  result = total_distance
  study = study
[]

[Outputs]
  checkpoint = true
  csv = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
