[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]
[Reporters]
  # [scc_crack_growth]
  #   type = ParsedVectorReporter
  #   name = crack_growth
  #   reporter_names = 'II_KI_1/II_KI_1'
  #   reporter_symbols = 'ki'
  #   scalar_reporter_names = 'dt/value'
  #   scalar_reporter_symbols = 'dt'
  #   constant_names = 'constant1 constant2'
  #   constant_expressions = '10 20'
  #   expression = 'if(ki<6.7e6,2e-9,if(ki>59e6,2.01e-7,3.33e-11*ki^2.161 ))'
  #   execute_on = 'XFEM_MARK TIMESTEP_END'
  # []
  [scc_crack_growth]
    type = ParsedVectorReporter
    name = crack_growth
    reporter_names = 'II_KI_1/II_KI_1'
    reporter_symbols = 'ki'
    scalar_reporter_names = 'dt/value'
    scalar_reporter_symbols = 'dt'
    constant_names = 'constant1 constant2'
    constant_expressions = '10 20'
    expression = 'if(ki<6.7e6,2e-9*dt,if(ki>59e6,2.01e-7*dt,(3.33e-11*ki^2.161)*dt ))'
    execute_on = 'XFEM_MARK TIMESTEP_END'
  []
[]
[UserObjects]
  [cut_mesh]
    type = CrackMeshCut3DUserObject
    mesh_generator_name = mesh_cutter
    growth_dir_method = MAX_HOOP_STRESS
    size_control = 1
    n_step_growth = 1
    growth_rate_method = REPORTER
    growth_reporter = "scc_crack_growth/crack_growth"
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

#functino for BC on top surface
[Functions]
  [top_trac_y]
    type = ConstantFunction
    value = 10
  []
  [top_trac_x]
    type = ConstantFunction
    value = 0
  []
[]

[Outputs]
  file_base = edge_crack_3d_scc_out
  json = true
  [xfemcutter]
    type = XFEMCutMeshOutput
    xfem_cutter_uo = cut_mesh
  []
[]
