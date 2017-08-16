[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  elem_type = HEX8
[]

[Variables]
  [./ux]
    block = 0
  [../]
  [./uy]
    block = 0
  [../]
  [./uz]
    block = 0
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./fp_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./rotout]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./e_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./gss1]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[Functions]
  [./tdisp]
    type = ParsedFunction
    value = 0.01*t
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 3
    index_i = 3
    execute_on = timestep_end
    block = 0
  [../]
  [./fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = fp
    index_j = 3
    index_i = 3
    execute_on = timestep_end
    block = 0
  [../]
  [./e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = lage
    index_j = 3
    index_i = 3
    execute_on = timestep_end
    block = 0
  [../]
  [./rotout]
    type = CrystalPlasticityRotationOutAux
    variable = rotout
    execute_on = timestep_end
    block = 0
  [../]
  [./gss1]
    type = CrystalPlasticitySlipSysAux
    variable = gss1
    slipsysvar = gss
    index_i = 1
    execute_on = timestep_end
    block = 0
  [../]
[]

[BCs]
  [./symmy]
    type = PresetBC
    variable = uy
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = PresetBC
    variable = ux
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = PresetBC
    variable = uz
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionPresetBC
    variable = uz
    boundary = front
    function = tdisp
  [../]
[]

[Materials]
  active = 'crysp'
  [./crysp]
    type = FiniteStrainCrystalPlasticity
    block = 0
    disp_y = uy
    disp_x = ux
    gtol = 1e-2
    slip_sys_file_name = input_slip_sys.txt
    disp_z = uz
    flowprops = '1 12 0.001 0.1'
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    nss = 12
    hprops = '1 541.5 60.8 109.8'
    gprops = '1 12 60.8'
    fill_method = symmetric9
    euler_angle_file_name = euler_ang_test.inp
  [../]
  [./elastic]
    type = FiniteStrainElasticMaterial
    block = 0
    disp_y = uy
    disp_x = ux
    disp_z = uz
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./fp_zz]
    type = ElementAverageValue
    variable = fp_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./e_zz]
    type = ElementAverageValue
    variable = e_zz
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./gss1]
    type = ElementAverageValue
    variable = gss1
    block = 'ANY_BLOCK_ID 0'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # Preconditioned JFNK (default)
  type = Transient
  dt = 0.05
  solve_type = PJFNK
  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  nl_abs_tol = 1e-10
  nl_rel_step_tol = 1e-10
  dtmax = 0.05
  nl_rel_tol = 1e-10
  ss_check_tol = 1e-10
  end_time = 1
  dtmin = 0.05
  nl_abs_step_tol = 1e-10
[]

[Outputs]
  file_base = outpoly
  exodus = true
[]

[TensorMechanics]
  [./solid]
    disp_z = uz
    disp_y = uy
    disp_x = ux
  [../]
[]
