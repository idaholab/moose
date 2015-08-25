[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
  displacements = 'ux uy uz'
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

[Kernels]
  [./TensorMechanics]
    disp_x = ux
    disp_z = uz
    disp_y = uy
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./peeq_soft]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./peeq_hard]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
  [./fp_zz]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
    block = 0
  [../]
  [./fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = fp
    index_j = 2
    index_i = 2
    execute_on = timestep_end
    block = 0
  [../]
  [./peeq_soft]
    type = MaterialStdVectorAux
    variable = peeq_soft
    property = internal_var
    index = 0
    execute_on = timestep_end
    block = 0
  [../]
  [./peeq_hard]
    type = MaterialStdVectorAux
    variable = peeq_hard
    property = internal_var
    index = 1
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
    function = '0.01*t'
  [../]
[]

[UserObjects]
  [./stress_uo]
    type = HyperElasticStress
  [../]
  [./flowstress_soft]
    type = RambergOsgoodHardening
    yield_stress = 100
    hardening_exponent = 0.1
    reference_plastic_strain = 0.002
  [../]
  [./flowstress_hard]
    type = RambergOsgoodHardening
    yield_stress = 100
    hardening_exponent = 0.3
    reference_plastic_strain = 0.002
  [../]
  [./flowrate_soft]
    type = FlowRateModel
    reference_flow_rate = 0.0001
    flow_rate_exponent = 50.0
    flow_rate_tol = 1
    flow_stress_user_object = flowstress_soft
  [../]
  [./flowrate_hard]
    type = FlowRateModel
    reference_flow_rate = 0.0001
    flow_rate_exponent = 50.0
    flow_rate_tol = 1
    flow_stress_user_object = flowstress_hard
  [../]
[]

[Materials]
  [./strain]
    type = ComputeFiniteStrain
    block = 0
    displacements = 'ux uy uz'
  [../]
  [./viscop]
    type = FiniteStrainHyperElasticViscoPlastic
    block = 0
    resid_abs_tol = 1e-18
    resid_rel_tol = 1e-8
    maxiters = 50
    max_substep_iteration = 5
    stress_user_object = stress_uo
    flow_rate_user_objects = 'flowrate_soft flowrate_hard'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '2.8e5 1.2e5 1.2e5 2.8e5 1.2e5 2.8e5 0.8e5 0.8e5 0.8e5'
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
  [./peeq_soft]
    type = ElementAverageValue
    variable = peeq_soft
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./peeq_hard]
    type = ElementAverageValue
    variable = peeq_hard
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
  type = Transient
  dt = 0.02

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = -pc_hypre_type
  petsc_options_value = boomerang
  dtmax = 10.0
  nl_rel_tol = 1e-10
  dtmin = 0.02
  num_steps = 10
[]

[Outputs]
  file_base = one_elem_multi
  exodus = true
  csv = false
  print_linear_residuals = true
[]

[Problem]
  use_legacy_uo_initialization = false
[]
