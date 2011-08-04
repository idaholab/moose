[Mesh]
#  dim =  2
  file =  2d_stoch_aperture_caco3_pe1.5_64_mesh_out.e
#  uniform_refine = 6
[]

[Variables]
  active =  'pressure tracer ca2+ h+ hco3-'	

  [./pressure]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'pressure'
    initial_from_file_timestep = 51     
#     [./InitialCondition]
#     type = BoundingBoxIC
#     x1 = 0.0
#     y1 = 0.0
#     x2 = 1.0e-10
#     y2 = 0.15
#     inside = 60.0
#     outside = 0.0
#     [../]
  [../]

  [./tracer]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'tracer'
    initial_from_file_timestep = 51     
#     [./InitialCondition]
#     type = BoundingBoxIC
#     x1 = 0.0
#     y1 = 0.0
#     x2 = 1.0e-10
#     y2 = 0.15
#     inside = 1.0
#     outside = 0.0
#     [../]
  [../]

  [./ca2+]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'ca2+'
    initial_from_file_timestep = 51     
#     [./InitialCondition]
#     type = BoundingBoxIC
#     x1 = 0.0
#     y1 = 0.0
#     x2 = 1.0e-10
#     y2 = 0.15
#     inside = 1.0e-6
#     outside = 5.0e-2
#     [../]
  [../]

  [./h+]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'h+'
    initial_from_file_timestep = 51    
#    initial_condition = 1.0e-7
  [../]

  [./hco3-]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'hco3-'
    initial_from_file_timestep = 51    
#    initial_condition = 1.0e-6
#     [./InitialCondition]
#     type = BoundingBoxIC
#     x1 = 0.0
#     y1 = 0.0
#     x2 = 1.0e-10
#     y2 = 0.15
#     inside = 5.0e-2
#     outside = 1.0e-6
#     [../]
  [../]

[]

[AuxVariables]
   active =  'conductivity caco3(s)'
  [./conductivity]
    order =  CONSTANT
    family =  MONOMIAL
  [../]
  [./caco3(s)]
    order =  FIRST
    family =  LAGRANGE
    initial_from_file_var = 'caco3(s)'
    initial_from_file_timestep = 51     
  [../]

[]

[Kernels]
#  active = 'pressure_ie          pressure_diff

#            tracer_ie            tracer_pd           tracer_conv

#            ca2+_ie              ca2+_pd             ca2+_conv 
#            ca2+_caco3(aq)_sub   ca2+_caco3(aq)_cd   ca2+_caco3(aq)_conv
# 	   ca2+_cahco3+_sub     ca2+_cahco3+_cd     ca2+_cahco3+_conv
# 	   ca2+_caoh+_sub       ca2+_caoh+_cd       ca2+_caoh+_conv
#            ca2+_calcite 

#            h+_ie                h+_pd               h+_conv
# 	   h+_caco3(aq)_sub     h+_caco3(aq)_cd     h+_caco3(aq)_conv
# 	   h+_caoh+_sub         h+_caoh+_cd         h+_caoh+_conv
# 	   h+_co2(aq)_sub       h+_co2(aq)_cd       h+_co2(aq)_conv
# 	   h+_co32-_sub         h+_co32-_cd         h+_co32-_conv
# 	   h+_oh-_sub           h+_oh-_cd           h+_oh-_conv
# 	   h+_calcite

#            hco3-_ie             hco3-_pd            hco3-_conv
#            hco3-_caco3(aq)_sub  hco3-_caco3(aq)_cd  hco3-_caco3(aq)_conv
#            hco3-_cahco3+_sub    hco3-_cahco3+_cd    hco3-_cahco3+_conv
#            hco3-_co2(aq)_sub    hco3-_co2(aq)_cd    hco3-_co2(aq)_conv
#            hco3-_co32-_sub      hco3-_co32-_cd      hco3-_co32-_conv
# 	   hco3-_calcite'

 active = 'pressure_ie          pressure_diff

           tracer_ie            tracer_pd           tracer_conv

           ca2+_ie              ca2+_pd             ca2+_conv 
           ca2+_caco3(aq)_sub   ca2+_caco3(aq)_cd   ca2+_caco3(aq)_conv
	   ca2+_cahco3+_sub     ca2+_cahco3+_cd     ca2+_cahco3+_conv
           ca2+_calcite 

           h+_ie                h+_pd               h+_conv
 	   h+_caco3(aq)_sub     h+_caco3(aq)_cd     h+_caco3(aq)_conv
	   h+_calcite

           hco3-_ie             hco3-_pd            hco3-_conv
           hco3-_caco3(aq)_sub  hco3-_caco3(aq)_cd  hco3-_caco3(aq)_conv
           hco3-_cahco3+_sub    hco3-_cahco3+_cd    hco3-_cahco3+_conv
	   hco3-_calcite'

# Pressure
  [./pressure_ie]
    type =  PressureImplicitEuler
    variable =  pressure
  [../]

  [./pressure_diff]
    type =  PressureDiffusion
    variable =  pressure
  [../]

# tracer
  [./tracer_ie]
    type =  PrimaryImplicitEuler
    variable =  tracer
  [../]

  [./tracer_pd]
    type =  PrimaryDiffusion
    variable =  tracer
  [../]

  [./tracer_conv]
    type =  PrimaryConvection
    variable =  tracer
    p =  'pressure'
  [../]

# ca2+
  [./ca2+_ie]
    type =  PrimaryImplicitEuler
    variable =  ca2+
  [../]

  [./ca2+_pd]
    type =  PrimaryDiffusion
    variable =  ca2+
  [../]

  [./ca2+_conv]
    type =  PrimaryConvection
    variable =  ca2+
    p =  'pressure'
  [../]

  [./ca2+_caco3(aq)_sub]
    type =  CoupledBEEquilibriumSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v = '-1.0 1.0'
    v =  'h+ hco3-'
    # start_time = 1.0
  [../]

  [./ca2+_caco3(aq)_cd]
    type =  CoupledDiffusionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v = '-1.0 1.0'
    v =  'h+ hco3-'
    # start_time = 1.0
  [../]

  [./ca2+_caco3(aq)_conv]
    type =  CoupledConvectionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v = '-1.0 1.0'
    v =  'h+ hco3-'
    p =  'pressure'
    # start_time = 1.0
  [../]

  [./ca2+_cahco3+_sub]
    type =  CoupledBEEquilibriumSub
    variable =  ca2+
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./ca2+_cahco3+_cd]
    type =  CoupledDiffusionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./ca2+_cahco3+_conv]
    type =  CoupledConvectionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
    p =  'pressure'
  [../]

  [./ca2+_caoh+_sub]
    type =  CoupledBEEquilibriumSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -12.85
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
  [../]

  [./ca2+_caoh+_cd]
    type =  CoupledDiffusionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -12.85
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
  [../]

  [./ca2+_caoh+_conv]
    type =  CoupledConvectionReactionSub
    variable =  ca2+
    weight =  1.0                
    log_k  = -12.85
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
    p =  'pressure'
  [../]

  [./ca2+_calcite]
    type =  CoupledBEKinetic
    variable =  ca2+
    weight =  1.0
    v =  'caco3(s)'
    # start_time = 1.0
  [../]

# h+
  [./h+_ie]
    type =  PrimaryImplicitEuler
    variable =  h+
  [../]

  [./h+_pd]
    type =  PrimaryDiffusion
    variable =  h+
  [../]

  [./h+_conv]
    type =  PrimaryConvection
    variable =  h+
    p =  'pressure'
  [../]

  [./h+_caco3(aq)_sub]
    type =  CoupledBEEquilibriumSub
    variable =  h+
    weight = -1.0                
    log_k  = -7.009
    sto_u = -1.0 
    sto_v =  '1.0 1.0'
    v =  'ca2+ hco3-'
    # start_time = 1.0
  [../]

  [./h+_caco3(aq)_cd]
    type =  CoupledDiffusionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -7.009
    sto_u = -1.0 
    sto_v =  '1.0 1.0'
    v =  'ca2+ hco3-'
    # start_time = 1.0
  [../]

  [./h+_caco3(aq)_conv]
    type =  CoupledConvectionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -7.009
    sto_u = -1.0 
    sto_v =  '1.0 1.0'
    v =  'ca2+ hco3-'
    p =  'pressure'
    # start_time = 1.0
  [../]

  [./h+_caoh+_sub]
    type =  CoupledBEEquilibriumSub
    variable =  h+
    weight = -1.0                
    log_k  = -12.85
    sto_u = -1.0 
    sto_v =  1.0
    v =  'ca2+'
  [../]

  [./h+_caoh+_cd]
    type =  CoupledDiffusionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -12.85
    sto_u = -1.0 
    sto_v =  1.0
    v =  'ca2+'
  [../]

  [./h+_caoh+_conv]
    type =  CoupledConvectionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -12.85
    sto_u = -1.0 
    sto_v =  1.0
    v =  'ca2+'
    p =  'pressure'
  [../]

  [./h+_co2(aq)_sub]
    type =  CoupledBEEquilibriumSub
    variable =  h+
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./h+_co2(aq)_cd]
    type =  CoupledDiffusionReactionSub
    variable =  h+
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./h+_co2(aq)_conv]
    type =  CoupledConvectionReactionSub
    variable =  h+
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0 
    sto_v =  1.0
    v =  'hco3-'
    p =  'pressure'
  [../]

  [./h+_co32-_sub]
    type =  CoupledBEEquilibriumSub
    variable =  h+
    weight = -1.0                
    log_k  = -10.325
    sto_u = -1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./h+_co32-_cd]
    type =  CoupledDiffusionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -10.325
    sto_u = -1.0 
    sto_v =  1.0
    v =  'hco3-'
  [../]

  [./h+_co32-_conv]
    type =  CoupledConvectionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -10.325
    sto_u = -1.0 
    sto_v =  1.0
    v =  'hco3-'
    p =  'pressure'
  [../]

  [./h+_oh-_sub]
    type =  CoupledBEEquilibriumSub
    variable =  h+
    weight = -1.0                
    log_k  = -13.991
    sto_u = -1.0 
    sto_v = ' ' 
  [../]

  [./h+_oh-_cd]
    type =  CoupledDiffusionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -13.991
    sto_u = -1.0 
    sto_v = ' ' 
  [../]

  [./h+_oh-_conv]
    type =  CoupledConvectionReactionSub
    variable =  h+
    weight = -1.0                
    log_k  = -13.991
    sto_u = -1.0 
    sto_v = ' ' 
    p =  'pressure'
  [../]

  [./h+_calcite]
    type =  CoupledBEKinetic
    variable =  h+
    weight =  -1.0 
    v =  'caco3(s)'
    # start_time = 1.0
  [../]

# hco3-
  [./hco3-_ie]
    type =  PrimaryImplicitEuler
    variable =  hco3-
  [../]

  [./hco3-_pd]
    type =  PrimaryDiffusion
    variable =  hco3-
  [../]

  [./hco3-_conv]
    type =  PrimaryConvection
    variable =  hco3-
    p =  'pressure'
  [../]

  [./hco3-_caco3(aq)_sub]
    type =  CoupledBEEquilibriumSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v =  '1.0  -1.0'
    v =  'ca2+ h+'
    # start_time = 1.0
  [../]

  [./hco3-_caco3(aq)_cd]
    type =  CoupledDiffusionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v =  '1.0  -1.0'
    v =  'ca2+ h+'
    # start_time = 1.0
  [../]

  [./hco3-_caco3(aq)_conv]
    type =  CoupledConvectionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -7.009
    sto_u =  1.0 
    sto_v =  '1.0  -1.0'
    v =  'ca2+ h+'
    p =  'pressure'
    # start_time = 1.0
  [../]

  [./hco3-_cahco3+_sub]
    type =  CoupledBEEquilibriumSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'ca2+'
  [../]

  [./hco3-_cahco3+_cd]
    type =  CoupledDiffusionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'ca2+'
  [../]

  [./hco3-_cahco3+_conv]
    type =  CoupledConvectionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  1.043
    sto_u =  1.0 
    sto_v =  1.0
    v =  'ca2+'
    p =  'pressure'
  [../]

  [./hco3-_co2(aq)_sub]
    type =  CoupledBEEquilibriumSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0 
    sto_v =  1.0
    v =  'h+'
  [../]

  [./hco3-_co2(aq)_cd]
    type =  CoupledDiffusionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0 
    sto_v =  1.0
    v =  'h+'
  [../]

  [./hco3-_co2(aq)_conv]
    type =  CoupledConvectionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  =  6.341
    sto_u =  1.0
    sto_v =  1.0
    v =  'h+'
    p =  'pressure'
  [../]

  [./hco3-_co32-_sub]
    type =  CoupledBEEquilibriumSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -10.325
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
  [../]

  [./hco3-_co32-_cd]
    type =  CoupledDiffusionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -10.325
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
  [../]

  [./hco3-_co32-_conv]
    type =  CoupledConvectionReactionSub
    variable =  hco3-
    weight =  1.0                
    log_k  = -10.325
    sto_u =  1.0 
    sto_v = -1.0
    v =  'h+'
    p =  'pressure'
  [../]

  [./hco3-_calcite]
    type =  CoupledBEKinetic
    variable =  hco3-
    weight =  1.0 
    v =  'caco3(s)'
    # start_time = 1.0
  [../]

[]

[AuxKernels]
  active =  'aux_conductivity aux_caco3(s)'

# conductivity
  [./aux_conductivity]
    type =  StochasticFieldAux
    variable =  conductivity
    file_name =  k_field.dat
  [../]

# caco3(s)
  [./aux_caco3(s)]
    type =  KineticDisPreConcAux
    variable =  caco3(s)
    log_k  =   1.8487
    sto_v =   '1.0 1.0  -1.0'
    r_area =   4.61e-4
    ref_kconst =  6.456542e-7
    e_act =  1.5e4
    gas_const =  8.314
    ref_temp =  298.15
    sys_temp =  298.15
    v =  'ca2+ hco3- h+'
    # start_time = 1.0
  [../]

[]

[BCs]
  active = 'pressure_left pressure_right
            tracer_left     tracer_right
            ca2+_left	  ca2+_right
	    hco3-_left	  hco3-_right
	    h+_left	  h+_right'

# pressure
  [./pressure_left]
    type =  DirichletBC
    variable =  pressure
    boundary =  1
    value =  60.0
  [../]

  [./pressure_right]
    type =  DirichletBC
    variable =  pressure
    boundary =  2
    value =  0.0
  [../]

# tracer
  [./tracer_left]
    type =  DirichletBC
    variable =  tracer
    boundary =  1
    value =  1.0
  [../]
  [./tracer_right]
    type =  OutFlowBC
    variable =  tracer
    boundary =  2
    porosity =  1.0
    diffusivity =  7.5e-7
  [../]

# ca2+
  [./ca2+_left]
   type =  DirichletBC
   variable =  ca2+
   boundary =  1
   value =  1.0e-6
#     type =  SinDirichletBC
#     variable =  ca2+
#     boundary =  1
#     initial = 5.0e-2
#     final = 1.0e-6
#     duration = 1
  [../]
  [./ca2+_right]
    type =  OutFlowBC
    variable =  ca2+
    boundary =  2
    porosity =  1.0
    diffusivity =  7.5e-7
  [../]

# hco3-
  [./hco3-_left]
   type =  DirichletBC
   variable =  hco3-
   boundary =  1
   value =  1.0e-2
#     type =  SinDirichletBC
#     variable =  hco3-
#     boundary =  1
#     initial = 1.0e-6
#     final = 5.0e-2
#     duration = 1
  [../]

  [./hco3-_right]
    type =  OutFlowBC
    variable =  hco3-
    boundary =  2
    porosity =  1.0
    diffusivity =  7.5e-7
  [../]

# h+
  [./h+_left]
    type =  DirichletBC
    variable =  h+
    boundary =  1
    value =  1.0e-7
  [../]

  [./h+_right]
    type =  OutFlowBC
    variable =  h+
    boundary =  2
    porosity =  1.0
    diffusivity =  7.5e-7
  [../]

[]

[Materials]
  active =  'stochastic'

  [./stochastic]
    type =  StochasticMaterial
    block =  1

    diffusivity      =  7.5e-7
    storage          =  1.0e-5
    init_porosity    =  1.0

    conductivity     =  conductivity

    mineral          =  0.00
    molecular_weight =  100.08
    density          =  2.7099
    v =  'caco3(s)'
  [../]
[]

[Executioner]
active = ' '

  type =  Transient
#  type =  SolutionTimeAdaptive
  perf_log =  true
  petsc_options =  '-snes_mf_operator'
  petsc_options_iname =  '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value =  'hypre    boomeramg      51'
  l_max_its =  50
  l_tol =  1e-5
  nl_max_its =  10
  nl_rel_tol =  1e-5
  start_time =  0.0
  end_time = 2.0
  num_steps =  10000000
  dt = 0.02
  dtmin =  0.0000001
  dtmax =  0.1
#  sol_time_adaptive_time_stepping =  true
   sync_times = '1.0 4.0 8.0 10.0'

  [./Adaptivity]
#    initial_adaptivity = 3
    error_estimator = KellyErrorEstimator
    refine_fraction = 0.85
    coarsen_fraction = 0.05
#    weight_names = 'urea nh4+'
#    weight_values = '1.0e2 1.0e6'
    max_h_level = 2
  [../]

[]

[Output]
  file_base = 2d_stoch_aperture_caco3_pe1.5_64_transient_out
  output_initial =  true
  interval =  1
  exodus =  true
[]
