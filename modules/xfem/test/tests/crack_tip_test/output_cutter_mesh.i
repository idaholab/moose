[Outputs]
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh2
  []
  execute_on = timestep_end
  [./console]
    type = Console
    output_linear = true
  [../]
[]
