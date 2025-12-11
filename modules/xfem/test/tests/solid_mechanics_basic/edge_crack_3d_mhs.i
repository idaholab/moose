[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = MAX_HOOP_STRESS
    size_control = 1
    n_step_growth = 1
    growth_rate = growth_func_v
    crack_front_nodes = '7 6 5 4'
  []
[]

[DomainIntegral]
  integrals = 'Jintegral InteractionIntegralKI InteractionIntegralKII'
  displacements = 'disp_x disp_y disp_z'
  crack_front_points_provider = cut_mesh
  number_points_from_provider = 4
  crack_direction_method = CurvedCrackFront
  radius_inner = '0.15'
  radius_outer = '0.45'
  poissons_ratio = 0.3
  youngs_modulus = 207000
  block = 0
  incremental = true
[]

# function for CrackMeshCut3DUserObject crack
[Functions]
  [growth_func_v]
    type = ParsedFunction
    expression = 0.15
  []
[]

#function for BC on top surface
[Functions]
  [top_trac_x]
    type = ConstantFunction
    value = 100
  []
  [top_trac_y]
    type = ConstantFunction
    value = 100
  []
[]

[Outputs]
  file_base = edge_crack_3d_mhs_out
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh
  []
[]
