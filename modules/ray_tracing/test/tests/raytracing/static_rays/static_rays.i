[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmax = 100
  ymax = 100
  zmax = 100
[]

[UserObjects/study]
  type = TestPICRayStudy
  start_points = '0 0 0
                  50 50 50
                  1 99 81
                  49 49 100'
  start_directions = '0 0 0
                      0 0 0
                      0 0 0
                      0 0 0'
  velocity_function = '0'
  allow_static_rays = true
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

[Outputs]
  [rays]
    type = RayTracingExodus
    study = study
    execute_on = TIMESTEP_BEGIN
  []
[]
