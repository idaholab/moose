[Mesh]
  file = indenter.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
  order = FIRST
  family = LAGRANGE
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
[]

[AuxKernels]
[]

[Functions]
  [./push_down]
    type = ParsedFunction
    expression = 'if(t < 1.5, -t, t-3.0)'
  [../]
[]


[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    use_automatic_differentiation = false
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
    save_in = 'saved_x saved_y saved_z'
    use_finite_deform_jacobian = true
  [../]
[]

[BCs]
  [./botz]
    type = DirichletBC
    variable = disp_z
    boundary = 101
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 101
    value = 0.0
  [../]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  [../]

  [./boty111]
    type = DirichletBC
    variable = disp_y
    boundary = 111
    value = 0.0
  [../]
  [./botx111]
    type = DirichletBC
    variable = disp_x
    boundary = 111
    value = 0.0
  [../]

  [./topz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '201'
    function = push_down
  [../]
  [./topy]
    type = DirichletBC
    variable = disp_y
    boundary = 201
    value = 0.0
  [../]
  [./topx]
    type = DirichletBC
    variable = disp_x
    boundary = 201
    value = 0.0
  [../]
[]


[UserObjects]
  [./slip_rate_gss]
    type = CrystalPlasticitySlipRateGSS
    variable_size = 48
    slip_sys_file_name = input_slip_sys_bcc48.txt
    num_slip_sys_flowrate_props = 2
    flowprops = '1 48 0.0001 0.01'
    uo_state_var_name = state_var_gss
    slip_incr_tol = 10.0
    block = 1
  [../]
  [./slip_resistance_gss]
    type = CrystalPlasticitySlipResistanceGSS
    variable_size = 48
    uo_state_var_name = state_var_gss
    block = 1
  [../]
  [./state_var_gss]
    type = CrystalPlasticityStateVariable
    variable_size = 48
    groups = '0 24 48'
    group_values = '900 1000' #120
    uo_state_var_evol_rate_comp_name = state_var_evol_rate_comp_gss
    scale_factor = 1.0
    block = 1
  [../]
  [./state_var_evol_rate_comp_gss]
    type = CrystalPlasticityStateVarRateComponentGSS
    variable_size = 48
    hprops = '1.4 1000 1200 2.5'
    uo_slip_rate_name = slip_rate_gss
    uo_state_var_name = state_var_gss
    block = 1
  [../]
[]

[Materials]
  [./crysp]
    type = FiniteStrainUObasedCP
    block = 1
    stol = 1e-2
    tan_mod_type = exact
    uo_slip_rates = 'slip_rate_gss'
    uo_slip_resistances = 'slip_resistance_gss'
    uo_state_vars = 'state_var_gss'
    uo_state_var_evol_rate_comps = 'state_var_evol_rate_comp_gss'
    maximum_substep_iteration = 25
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensorCP
    block = 1
    C_ijkl = '265190 113650 113650 265190 113650 265190 75769 75769 75760'
    fill_method = symmetric9
  [../]
  [./elasticity_tensor_indenter]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000000.0
    poissons_ratio = 0.3
    block = 2
  [../]
  [./stress_indenter]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 1
  [../]
  [./resid_z]
    type = NodalSum
    variable = saved_z
    boundary = 201
  [../]
  [./disp_z]
    type = NodalExtremeValue
    variable = disp_z
    boundary = 201
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu    superlu_dist'
  line_search = 'none'

  l_max_its = 60
  nl_max_its = 50
  dt = 0.004
  dtmin = 0.00001
  end_time = 1.8
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6 # 6 if no friction
  l_tol = 1e-3
  automatic_scaling = true

[]

[Outputs]
  [./my_checkpoint]
    type = Checkpoint
    time_step_interval = 50
  [../]
  exodus = true
  csv = true
  print_linear_residuals = true
  print_perf_log = true
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Dampers]
  [./contact_slip]
    type = ContactSlipDamper
    primary = '202'
    secondary = '102'
  [../]
[]

[Contact]
  [./ind_base]
    primary = 202
    secondary = 102
    model = coulomb
    friction_coefficient = 0.4
    normalize_penalty = true
    formulation = tangential_penalty
    penalty = 1e7
    capture_tolerance = 0.0001
  [../]
[]
