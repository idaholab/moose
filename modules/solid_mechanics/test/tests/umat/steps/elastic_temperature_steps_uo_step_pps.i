[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 10.0
  dt = 1.0
[]

[UserObjects]
  [step_uo]
   type = StepUserObject
   step_start_times = '0 5'
  []
[]

[Postprocessors]
  [step_number]
    type = StepNumber
    step_user_object = step_uo
  []
[]

[Outputs]
  csv = true
[]
