!include problem.i

[Executioner]
  type = MFEMTransient
  num_steps = 10
  dt = 0.2
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/dt_from_parent
    vtk_format = ASCII
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    input_files = dt_from_parent.i
    # dt will be constrained by the parent solve
    cli_args = 'Executioner/dt=1;Outputs/ParaViewDataCollection/file_base=OutputData/dt_from_parent_sub;MultiApps/active='
  []
[]
