[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 3
  ny = 7
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Functions]
  [data1]
    type = ParsedFunction
    expression = 'x + y'
  []
  [data2]
    type = ParsedFunction
    expression = 'x * (t + 1)'
  []
  [auxdata1]
    type = ParsedFunction
    expression = 'x * y + t'
  []
[]

[UserObjects/study]
  type = StationaryRayStudyTest
  always_cache_traces = true
  data_on_cache_traces = true
  aux_data_on_cache_traces = true
  ray_kernel_coverage_check = false
  data_functions = 'data1 data2'
  aux_data_functions = 'auxdata1'
  execute_on = timestep_end
[]

[Outputs/rays]
  type = RayTracingExodus
  study = study
  execute_on = timestep_end
  output_data = true
  output_aux_data = true
  output_properties = ''
[]
