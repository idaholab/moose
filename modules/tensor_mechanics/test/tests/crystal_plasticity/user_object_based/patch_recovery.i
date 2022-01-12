[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  displacements = 'ux uy'
[]

[Variables]
  [ux]
  []
  [uy]
  []
[]

[AuxVariables]
  [stress_xx_recovered]
    order = FIRST
    family = LAGRANGE
  []
  [stress_yy_recovered]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [tdisp]
    type = ParsedFunction
    value = 0.01*t
  []
[]

[Kernels]
  [TensorMechanics]
    displacements = 'ux uy'
    use_displaced_mesh = true
  []
[]

[AuxKernels]
  [stress_xx_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_xx_recovered
    nodal_patch_recovery_uo = stress_xx_patch
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_yy_recovered
    nodal_patch_recovery_uo = stress_yy_patch
    execute_on = 'TIMESTEP_END'
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
  [tdisp]
    type = FunctionDirichletBC
    variable = uy
    boundary = top
    function = tdisp
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
  [stress_xx_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '0 0'
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '1 1'
    execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [crysp]
    type = FiniteStrainUObasedCP
    stol = 1e-2
    tan_mod_type = exact
    uo_slip_rates = 'slip_rate_gss'
    uo_slip_resistances = 'slip_resistance_gss'
    uo_state_vars = 'state_var_gss'
    uo_state_var_evol_rate_comps = 'state_var_evol_rate_comp_gss'
  []
  [strain]
    type = ComputeFiniteStrain
    displacements = 'ux uy'
  []
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
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
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  dtmax = 10.0
  nl_rel_tol = 1e-10
  end_time = 1
  dtmin = 0.05
  num_steps = 10
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
