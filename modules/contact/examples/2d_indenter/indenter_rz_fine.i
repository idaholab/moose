[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
[]

[Mesh]
  patch_update_strategy = auto
  patch_size = 2
  partitioner = centroid
  centroid_partitioner_direction = y

  [simple_mesh]
    type = FileMeshGenerator
    file = indenter_rz_fine_bigsideset.e
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '4'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
    input = simple_mesh
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    sidesets = '6'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
    input = secondary
  []

  # For NodalVariableValue to work with distributed mesh
  allow_renumbering = false
[../]

[Functions]
  [disp_y]
    type = PiecewiseLinear
    x = '0.  1.0     2.0    2.6   3.0'
    y = '0.  -4.5   -5.7   -5.7  -4.0'
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []

  [disp_y]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []

  [frictionless_normal_lm]
    block = 'secondary_lower'
  []
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = false
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
    save_in = 'saved_x saved_y'
  []
[]

[BCs]
  # Symmetries of the Problem
  [symm_x_indenter]
    type = DirichletBC
    variable = disp_x
    boundary = 5
    value = 0.0
  []

  [symm_x_material]
    type = DirichletBC
    variable = disp_x
    boundary = 9
    value = 0.0
  []

  # Material should not fly away
  [material_base_y]
    type = DirichletBC
    variable = disp_y
    boundary = 8
    value = 0.0
  []

  # Drive indenter motion
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = disp_y
  []
[]

[Constraints]
  # All constraints below for mechanical contact (Mortar)
  [lm]
    type = NormalNodalLMMechanicalContact
    secondary = '4'
    primary = 6
    variable = frictionless_normal_lm
    primary_variable = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    tangential_tolerance = 0.01
  []
  [x]
    type = NormalMortarMechanicalContact
    primary_boundary = '6'
    secondary_boundary = '4'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = frictionless_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = true
  []
  [y]
    type = NormalMortarMechanicalContact
    primary_boundary = '6'
    secondary_boundary = '4'
    primary_subdomain = '10000'
    secondary_subdomain = '10001'
    variable = frictionless_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = true
  []
[../]

[UserObjects]
  [slip_rate_gss]
    type = CrystalPlasticitySlipRateGSS
    variable_size = 48
    slip_sys_file_name = input_slip_sys_bcc48.txt
    num_slip_sys_flowrate_props = 2
    flowprops = '1 48 0.0001 0.01'
    uo_state_var_name = state_var_gss
    slip_incr_tol = 10.0
    block = 2
  []
  [slip_resistance_gss]
    type = CrystalPlasticitySlipResistanceGSS
    variable_size = 48
    uo_state_var_name = state_var_gss
    block = 2
  []
  [state_var_gss]
    type = CrystalPlasticityStateVariable
    variable_size = 48
    groups = '0 24 48'
    group_values = '900 1000' #120
    uo_state_var_evol_rate_comp_name = state_var_evol_rate_comp_gss
    scale_factor = 1.0
    block = 2
  []
  [state_var_evol_rate_comp_gss]
    type = CrystalPlasticityStateVarRateComponentGSS
    variable_size = 48
    hprops = '1.4 1000 1200 2.5'
    uo_slip_rate_name = slip_rate_gss
    uo_state_var_name = state_var_gss
    block = 2
  []
[]

[Materials]
  [tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e7
    poissons_ratio = 0.25
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []

  [crysp]
    type = FiniteStrainUObasedCP
    block = 2
    stol = 1e-2
    tan_mod_type = exact
    uo_slip_rates = 'slip_rate_gss'
    uo_slip_resistances = 'slip_resistance_gss'
    uo_state_vars = 'state_var_gss'
    uo_state_var_evol_rate_comps = 'state_var_evol_rate_comp_gss'
    maximum_substep_iteration = 20
  []
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    block = 2
    C_ijkl = '265190 113650 113650 265190 113650 265190 75769 75769 75760'
    fill_method = symmetric9
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'

  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type '
                        '-pc_factor_shift_amount'
  petsc_options_value = 'lu       basic                 NONZERO               1e-15'
  line_search = 'none'
  automatic_scaling = true
  nl_abs_tol = 1.5e-07
  nl_rel_tol = 1.5e-07
  l_max_its = 40
  l_abs_tol = 1e-08
  l_tol = 1e-08
  start_time = 0.0
  dt = 0.01
  end_time = 3.0 # Executioner
[]

[Postprocessors]
  [maxdisp]
    type = NodalVariableValue
    nodeid = 39
    variable = disp_y
  []
  [resid_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  []
[]

[Outputs]
  [out]
    type = Exodus
    elemental_as_nodal = true
  []
  perf_graph = true
  csv = true
[]
