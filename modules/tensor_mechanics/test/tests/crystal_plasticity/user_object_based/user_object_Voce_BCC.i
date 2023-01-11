[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4
  displacements = 'disp_x disp_y'
  nx = 2
  ny = 2
[]

[GlobalParams]
  volumetric_locking_correction = true
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fp_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./gss]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    expression = 0.01*t
  [../]
[]

[UserObjects]
  [./prop_read]
    type = PropertyReadFile
    prop_file_name = 'euler_ang_file.txt'
    # Enter file data as prop#1, prop#2, .., prop#nprop
    nprop = 3
    read_type = element
  [../]
[]

[AuxKernels]
  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./e_yy]
    type = RankTwoAux
    variable = e_yy
    rank_two_tensor = lage
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./fp_yy]
    type = RankTwoAux
    variable = fp_yy
    rank_two_tensor = fp
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./gss]
    type = MaterialStdVectorAux
    variable = gss
    property = state_var_gss
    index = 0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = tdisp
  [../]
[]

[UserObjects]
  [./slip_rate_gss]
    type = CrystalPlasticitySlipRateGSS
    variable_size = 48
    slip_sys_file_name = input_slip_sys_bcc48.txt
    num_slip_sys_flowrate_props = 2
    flowprops = '1 12 0.001 0.1 13 24 0.001 0.1 25 48 0.001 0.1'
    uo_state_var_name = state_var_gss
  [../]
  [./slip_resistance_gss]
    type = CrystalPlasticitySlipResistanceGSS
    variable_size = 48
    uo_state_var_name = state_var_gss
  [../]
  [./state_var_gss]
    type = CrystalPlasticityStateVariable
    variable_size = 48
    groups = '0 12 24 48'
    group_values =  '50 51 52'
    uo_state_var_evol_rate_comp_name = state_var_evol_rate_comp_voce
    scale_factor = 1.0
  [../]
  [./state_var_evol_rate_comp_voce]
    type = CrystalPlasticityStateVarRateComponentVoce
    variable_size = 48
    crystal_lattice_type = 'BCC'
    groups = '0 12 24 48'
    h0_group_values = '1 2 3'
    tau0_group_values = '50 51 52'
    tauSat_group_values = '70 81 92'
    hardeningExponent_group_values = '1 2 3'
    selfHardening_group_values ='4 5 6'
    coplanarHardening_group_values='7 8 9'
    GroupGroup_Hardening_group_values = '10 20 30
                                         40 50 60
                                         70 80 90'
    uo_slip_rate_name = slip_rate_gss
    uo_state_var_name = state_var_gss
  [../]
[]

[Materials]
  [./crysp]
    type = FiniteStrainUObasedCP
    stol = 1e-2
    tan_mod_type = exact
    uo_slip_rates = 'slip_rate_gss'
    uo_slip_resistances = 'slip_resistance_gss'
    uo_state_vars = 'state_var_gss'
    uo_state_var_evol_rate_comps = 'state_var_evol_rate_comp_voce'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
    read_prop_user_object = prop_read
  [../]
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./e_yy]
    type = ElementAverageValue
    variable = e_yy
  [../]
  [./fp_yy]
    type = ElementAverageValue
    variable = fp_yy
  [../]
  [./gss]
    type = ElementAverageValue
    variable = gss
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.01
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  dtmax = 10.0
  nl_rel_tol = 1e-10
  end_time = 1
  dtmin = 0.01
  num_steps = 10
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
