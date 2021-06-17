[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'left right'
[]

[UserObjects/study]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_rays_started]
    type = RayTracingStudyResult
    study = study
    result = total_rays_started
  []
  [total_processor_crossings]
    type = RayTracingStudyResult
    study = study
    result = total_processor_crossings
  []
  [max_processor_crossings]
    type = RayTracingStudyResult
    study = study
    result = max_processor_crossings
  []
  [total_distance]
    type = RayTracingStudyResult
    study = study
    result = total_distance
  []
[]

[Outputs]
  csv = true
[]
