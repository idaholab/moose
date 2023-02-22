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
  [step_uo]
   type = StepUserObject
   step_start_times = '-1 0 0.2 0.4'
  []
  [uexternaldb]
    type = AbaqusUExternalDB
    plugin = ../../plugins/utility_functions
    execute_on = 'INITIAL TIMESTEP_END TIMESTEP_BEGIN FINAL'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]
