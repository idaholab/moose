[Problem]
  solve = false
[]

[Mesh] #dummy
  type = GeneratedMesh 
  dim = 2
[]

[Executioner]
  type = Transient
  start_time = 0.0
  dt = 1 
  end_time = 10.0
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_end
    input_files = material_sub_app_test_sub.i
    sub_cycling = true
  [../]
[]

[Outputs]
  csv = false
  exodus = false
  color = false
  [./console]
    type = Console
    perf_log = true
    output_linear = false
    max_rows = 15
  [../]
[]
