!include problem.i

[Executioner]
  type = MFEMTransient
  num_steps = 10
  dt = 0.2
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = TIMESTEP_END
    file_base = dt_from_parent
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    input_files = dt_from_parent.i
    # dt will be constrained by the parent solve
    cli_args = 'Executioner/dt=1;Outputs/CSV/file_base=dt_from_parent_sub;MultiApps/active='
  []
[]
