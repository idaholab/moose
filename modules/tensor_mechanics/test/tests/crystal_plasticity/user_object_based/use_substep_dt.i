[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
  elem_type = HEX8
  displacements = 'ux uy uz'
[]

[Variables]
  [ux]
  []
  [uy]
  []
  [uz]
  []
[]

[AuxVariables]
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [pk2]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [rotout]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [gss]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_increment]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [TensorMechanics]
    displacements = 'ux uy uz'
    use_displaced_mesh = true
  []
[]

[AuxKernels]
  [stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [pk2]
    type = RankTwoAux
    variable = pk2
    rank_two_tensor = pk2
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = fp
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = lage
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [gss]
    type = MaterialStdVectorAux
    variable = gss
    property = state_var_gss
    index = 0
    execute_on = timestep_end
  []
  [slip_inc]
    type = MaterialStdVectorAux
    variable = slip_increment
    property = slip_rate_gss
    index = 0
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = uy
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = ux
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = uz
    boundary = back
    value = 0
  []
  [pushy]
    type = FunctionDirichletBC
    variable = uy
    boundary = top
    function = '-0.1*t'
  []
  [pullz]
    type = FunctionDirichletBC
    variable = uz
    boundary = front
    function = '0.1*t'
  []
[]

[UserObjects]
  [slip_rate_gss]
    type = CrystalPlasticitySlipRateGSS
    variable_size = 12
    slip_sys_file_name = input_slip_sys.txt
    num_slip_sys_flowrate_props = 2
    flowprops = '1 4 0.001 0.1 5 8 0.001 0.1 9 12 0.001 0.1'
    uo_state_var_name = state_var_gss
  []
  [slip_resistance_gss]
    type = CrystalPlasticitySlipResistanceGSS
    variable_size = 12
    uo_state_var_name = state_var_gss
  []
  [state_var_gss]
    type = CrystalPlasticityStateVariable
    variable_size = 12
    groups = '0 4 8 12'
    group_values = '60.8 60.8 60.8'
    uo_state_var_evol_rate_comp_name = state_var_evol_rate_comp_gss
    scale_factor = 1.0
  []
  [state_var_evol_rate_comp_gss]
    type = CrystalPlasticityStateVarRateComponentGSS
    variable_size = 12
    hprops = '1.0 541.5 109.8 2.5'
    uo_slip_rate_name = slip_rate_gss
    uo_state_var_name = state_var_gss
  []
[]

[Materials]
  [crysp]
    type = FiniteStrainUObasedCP
    block = 0
    stol = 1e-2
    tan_mod_type = exact
    uo_slip_rates = 'slip_rate_gss'
    uo_slip_resistances = 'slip_resistance_gss'
    uo_state_vars = 'state_var_gss'
    uo_state_var_evol_rate_comps = 'state_var_evol_rate_comp_gss'
  []
  [strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'ux uy uz'
  []
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    block = 0
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [pk2]
    type = ElementAverageValue
    variable = pk2
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [e_zz]
    type = ElementAverageValue
    variable = e_zz
  []
  [gss]
    type = ElementAverageValue
    variable = gss
  []
  [slip_increment]
    type = ElementAverageValue
    variable = slip_increment
  []
  [uy_avg_top]
    type = SideAverageValue
    variable = uy
    boundary = top
  []
  [uz_avg_front]
    type = SideAverageValue
    variable = uz
    boundary = front
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 0.05
  solve_type = 'NEWTON'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       superlu_dist'

  line_search = 'none'
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  dtmax = 10.0
  nl_rel_tol = 1e-10
  end_time = 1.0
  num_steps = 5
  dtmin = 0.001
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  csv = true
[]
