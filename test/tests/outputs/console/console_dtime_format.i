[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  start_time = -1
  end_time = 1e5
  [TimeSteppers]
    [ts]
      type = FunctionDT
      function = 'if(t<0,0.3,if(t<60,3,if(t<3600,160,if(t<86400,8000,90000))))'
    []
  []
[]

[Outputs]
  [screen]
    type = Console
    verbose = true
    time_format = dtime
    time_precision = 6
    execute_on = 'failed nonlinear linear timestep_begin timestep_end'
  []
[]
