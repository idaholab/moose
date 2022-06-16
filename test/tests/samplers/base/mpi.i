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
  [./sample]
    type = TestSampler
    execute_on = 'initial'
  [../]
[]

[Postprocessors]
  [./test]
    type = SamplerTester
    sampler = sample
    test_type = MPI
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
