[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[UserObjects]
  [uexternaldb]
    type = AbaqusUExternalDB
    plugin = ../../plugins/mutex_test
    execute_on = 'INITIAL TIMESTEP_BEGIN FINAL'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
