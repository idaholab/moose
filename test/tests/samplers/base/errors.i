[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[Samplers]
  [sample]
    type = TestSampler
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [test]
    type = SamplerTester
    sampler = sample
    test_type = 'getGlobalSamples'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
