[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = 0
    xmax = 1
  []
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'left right'
[]

[UserObjects/study]
  type = TestReuseRaysStudy
  ray_kernel_coverage_check = false
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Postprocessors]
  [total_distance]
    type = RayTracingStudyResult
    study = study
    result = total_distance
  []
  [total_rays_started]
    type = RayTracingStudyResult
    study = study
    result = total_rays_started
  []
[]

[Outputs]
  csv = true
[]
