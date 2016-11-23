[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  num_steps = 100
  dt = 1
[]

[UserObjects]
  [./cur_Sim]
    type = StateSimRunner
    model_path = 'path/to/model'
    seed = 7;
  [../]
[]

[Postprocessors]
  [./runTest]
    type = StateSimTester
    test_type = SYNCTIMES
    state_sim_runner = cur_Sim
  [../]
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
