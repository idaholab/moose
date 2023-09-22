[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 10
  ny = 10
  xmax = 100
  ymax = 100
[]

[UserObjects/study]
  type = TestPICRayStudy
  start_points = '0 0 0
                  100 100 0
                  1 99 0
                  49 49 0'
  start_directions = '1 2 0
                      -2 -1 0
                      5 -1 0
                      1 1.1 0'
  velocity_function = '11 - t'
  execute_on = TIMESTEP_BEGIN
  always_cache_traces = true
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[Problem]
  solve = false
[]

[RayKernels/kernel]
  type = NullRayKernel
[]

[Outputs/rays]
  type = RayTracingExodus
  study = study
  execute_on = TIMESTEP_BEGIN
[]
