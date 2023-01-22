# MOOSE input file
# Written by Pierre-Clement Simon - Idaho National Laboratory
#
# Project:
# TRISO fuel fission gas transport: Silver diffusion in silicon carbide
#
# Published with:
# ---
#
# Phase Field Model:   Isotropic diffusion equation
# type:                Steady-State
# Grain structure:     Bicrystal with heterogeneous diffusion (higher in GBs than within grains)
# BCs:                 Periodic for AEH, flux and fix for direct method
# System:              Ag in SiC with bulk and Gb diffusion from LLS
#
#
# Info:
# - Dimentional input file for the diffusion of a solute in a complex
#   polycrystal
#
#
# Updates from previous file:
#
#
# Units
# length: nm
# time: s
# energy: --
# quantity: --


[Mesh]
  file = 'GB_Type_Phase1_out.e'
[]

[GlobalParams]
  op_num = 6
  var_name_base = gr
[]

[UserObjects]
  [./initial_grains]
    type = SolutionUserObject
    mesh = 'GB_Type_Phase1_out.e'
    timestep = LATEST
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.2
    connecting_threshold = 0.08
    compute_var_to_feature_map = true
    flood_entity_type = ELEMENTAL
    compute_halo_maps = true # For displaying HALO fields
  [../]
[]

[Variables]
  [./cx_AEH] #composition used for the x-component of the AEH solve
    initial_condition = 0.5
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
      variable = 'cx_AEH'
    [../]
  [../]
[]

[AuxVariables]
  [./gr0]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gr1]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gr2]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gr3]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gr4]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gr5]
    order = FIRST
    family = LAGRANGE
  [../]
  [./bnds]
    order = FIRST
    family = LAGRANGE
  [../]
  [./bnds_LAGB]
    order = FIRST
    family = LAGRANGE
  [../]
  [./bnds_HAGB]
    order = FIRST
    family = LAGRANGE
  [../]
  [./gb_type]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./EBSD_grain]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./init_grO]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr0
    solution = initial_grains
    from_variable = gr0
  [../]
  [./init_gr1]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr1
    solution = initial_grains
    from_variable = gr1
  [../]
  [./init_gr2]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr2
    solution = initial_grains
    from_variable = gr2
  [../]
  [./init_gr3]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr3
    solution = initial_grains
    from_variable = gr3
  [../]
  [./init_gr4]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr4
    solution = initial_grains
    from_variable = gr4
  [../]
  [./init_gr5]
    type = SolutionAux
    execute_on = INITIAL
    variable = gr5
    solution = initial_grains
    from_variable = gr5
  [../]
  [./init_EBSD_grain]
    type = SolutionAux
    execute_on = INITIAL
    variable = EBSD_grain
    solution = initial_grains
    from_variable = ebsd_numbers
  [../]
  [./gb_type]
    type = SolutionAux
    execute_on = 'INITIAL TIMESTEP_END'
    variable = gb_type
    solution = initial_grains
    from_variable = gb_type
  [../]
  [./bnds_aux]
    # AuxKernel that calculates the GB term
    type = BndsCalcAux
    variable = bnds
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./bnds_LAGB]
    # Calculate the bnds for specific GB type
    type = SolutionAuxMisorientationBoundary
    variable = bnds_LAGB
    gb_type_order = 1
    solution = initial_grains
    from_variable = gb_type
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./bnds_HAGB]
    # Calculate the bnds for specific GB type
    type = SolutionAuxMisorientationBoundary
    variable = bnds_HAGB
    gb_type_order = 2
    solution = initial_grains
    from_variable = gb_type
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]


[Kernels]
  [./Diff_x]
    type = MatDiffusion
    diffusivity = D_Scaling
    variable = cx_AEH
    args = 'bnds'
  [../]
[]

[Materials]
  #=========================================================== Generic Constants
  [./consts]
    type = GenericConstantMaterial
    prop_names =  'R            T   '
    prop_values = '8.3145       1450'
    # unit         J.mol-1.K-1  K
  [../]
  [./consts_expected]
    type = GenericConstantMaterial
    prop_names =  'Db          Dgbl     Dgbh'
    prop_values = '0.007       0.302    821.672'
    # unit         nm^2/s      nm^2/s   nm^2/s
    outputs = exodus
  [../]
  #===================================================== Interpolation functions
  [./hgb] # equal to 1 in grain boundaries, 0 elsewhere in grains.
    type = DerivativeParsedMaterial
    coupled_variables = 'bnds'
    constant_names =       'bnds_middle width tanh_cst_x2'
    constant_expressions = '0.75         0.0596 2.1972245773362196'
    expression = '1-0.5*(1.0+tanh(tanh_cst_x2*(bnds-bnds_middle)/width))'
    property_name = 'hgb'
    outputs = exodus
  [../]
  [./hgb_lagb] # equal to 1 in grain boundaries, 0 elsewhere in grains.
    type = DerivativeParsedMaterial
    coupled_variables = 'bnds_LAGB'
    constant_names =       'bnds_middle width tanh_cst_x2'
    constant_expressions = '0.75         0.0596 2.1972245773362196'
    expression = '1-0.5*(1.0+tanh(tanh_cst_x2*(bnds_LAGB-bnds_middle)/width))'
    property_name = 'hgb_lagb'
    outputs = exodus
  [../]
  [./hgb_hagb] # equal to 1 in grain boundaries, 0 elsewhere in grains.
    type = DerivativeParsedMaterial
    coupled_variables = 'bnds_HAGB'
    constant_names =       'bnds_middle width tanh_cst_x2'
    constant_expressions = '0.75         0.0596 2.1972245773362196'
    expression = '1-0.5*(1.0+tanh(tanh_cst_x2*(bnds_HAGB-bnds_middle)/width))'
    property_name = 'hgb_hagb'
    outputs = exodus
  [../]
  #====================================================== Diffusion coefficients
  #====================== Diffusion coefficients - Basic values and coefficients
  [./Grain_boundary_width] # size of grain boundaries in input polycrystal, as well as length scales for domain size
    type = GenericConstantMaterial
    prop_names =  'wGB_ref wGB  L  '
    prop_values = '1       6   9000'
    # unit         --           --  --  --
  [../]
  #============================================ Corrected Diffusion coefficients
  #========================================================= Analytical 1 - 1x1y
  [./Diffusion_coefficient_D]
    type = DerivativeParsedMaterial
    property_name = 'D_Scaling'
    coupled_variables = 'bnds'
    material_property_names = 'Db Dgbh Dgbl hgb_lagb(bnds_LAGB) hgb_hagb(bnds_HAGB) hgb(bnds)'
    expression = '(1-hgb)*Db+hgb*hgb_lagb/(hgb_lagb+hgb_hagb)*Dgbl+hgb*hgb_hagb/(hgb_lagb+hgb_hagb)*Dgbh'
    outputs = exodus
    derivative_order = 2
  [../]
[]

# It converges faster if all the residuals are at the same magnitude
[Debug]
  show_var_residual_norms = true
[../]

[Preconditioning]
  [./SMP]
    type = SMP
    off_diag_row = 'cx_AEH'
    off_diag_column = 'cx_AEH'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 31 0.7'
  l_max_its = 50
  nl_max_its = 50
  l_tol = 1e-04
  l_abs_tol = 1e-50
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
