[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Samplers]
  [sampler]
    type = TestSampler
    num_rows = 10000000
    num_cols = 1
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total]
    type = MemoryUsage
    mem_units = 'bytes'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [per_proc]
    type = MemoryUsage
    value_type = "average"
    mem_units = 'bytes'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_proc]
    type = MemoryUsage
    value_type = "max_process"
    mem_units = 'bytes'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [test]
    type = SamplerTester
    sampler = sampler
    test_type = 'getLocalSamples'
  []

[]

[Outputs]
  csv = true
  perf_graph = true
[]
