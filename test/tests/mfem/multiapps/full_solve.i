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
    file_base = full_solve_parent
  []
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    execute_on = timestep_begin
    input_files = full_solve.i
    keep_full_output_history = true
    cli_args = 'Executioner/dt=0.01;Outputs/CSV/file_base=full_solve_sub;MultiApps/active='
  []
[]
