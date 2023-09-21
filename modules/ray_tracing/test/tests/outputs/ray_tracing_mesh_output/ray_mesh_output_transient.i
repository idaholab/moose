[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 10
    ymax = 10
  []
[]

[UserObjects/study]
  type = TestTransientRaysStudy
  ray_kernel_coverage_check = false
  distance_function = '(t + x) / 5'
  boundary = 'bottom'
  always_cache_traces = true
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Problem]
  solve = false
[]

[Outputs/rays]
  type = RayTracingExodus
  study = study
  execute_on = TIMESTEP_END
  # would cause diffs on IDs
  output_properties = intersections
[]
