!include problem.i

[Executioner]
  type = MFEMTransient
  num_steps = 2
  dt = 0.1
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/sub_cycling_parent
    vtk_format = ASCII
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    execute_on = timestep_end
    input_files = sub_cycling.i
    sub_cycling = true
    # num_steps will be constrained by sub-cycling
    cli_args = 'Executioner/dt=0.01;Outputs/ParaViewDataCollection/file_base=OutputData/sub_cycling_sub;MultiApps/active='
  []
[]
