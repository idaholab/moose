
[XFEM]
  geometric_cut_userobjects = 'cut_mesh'
  qrule = volfrac
  output_cut_plane = true
[]

[Functions]
  [growth_func_x]
    type = ParsedFunction
    expression = 'if(x>0, 0.2, -0.2)'
    # expression = 'if(t<3,0,if(x>0, 0.2, -0.2))'
  []
  [growth_func_y]
    type = ParsedFunction
    expression = 0.0
  []
  [growth_func_z]
    type = ParsedFunction
    expression = 'if(t<3,0,0.1)'
  []
  [growth_func_v]
    type = ParsedFunction
    expression = 'if(t<3,0.02,0.015)'
  []
[]

[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    # growth_dir_method = MAX_HOOP_STRESS
    growth_dir_method = FUNCTION
    growth_direction_x = growth_func_x
    growth_direction_y = growth_func_y
    growth_direction_z = growth_func_z
    size_control = .05
    n_step_growth = 1
    growth_rate = growth_func_v
  []
[]

[Outputs]
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh
  []
  execute_on=final
[]
