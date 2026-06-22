!include problem.i

[Executioner]
  type = MFEMTransient
  num_steps = 2
  dt = 0.1
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = TIMESTEP_END
    file_base = sub_cycling_parent
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    execute_on = timestep_end
    input_files = sub_cycling.i
    sub_cycling = true
    # num_steps will be constrained by sub-cycling
    cli_args = 'Executioner/dt=0.01;Outputs/CSV/file_base=sub_cycling_sub;MultiApps/active='
  []
[]
