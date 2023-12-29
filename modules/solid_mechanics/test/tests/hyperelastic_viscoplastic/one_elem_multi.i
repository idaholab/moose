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
    displacements = 'ux uy uz'
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
    type = MaterialRealAux
    variable = peeq_soft
    property = ep_eqv1
    execute_on = timestep_end
    block = 0
  [../]
  [./peeq_hard]
    type = MaterialRealAux
    variable = peeq_hard
    property = ep_eqv2
    execute_on = timestep_end
    block = 0
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = uy
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = ux
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = DirichletBC
    variable = uz
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = uz
    boundary = front
    function = '0.01*t'
  [../]
[]

[UserObjects]
  [./flowstress1]
    type = HEVPRambergOsgoodHardening
    yield_stress = 100
    hardening_exponent = 0.1
    reference_plastic_strain = 0.002
    intvar_prop_name = ep_eqv1
  [../]
  [./flowstress2]
    type = HEVPRambergOsgoodHardening
    yield_stress = 100
    hardening_exponent = 0.3
    reference_plastic_strain = 0.002
    intvar_prop_name = ep_eqv2
  [../]
  [./flowrate1]
    type = HEVPFlowRatePowerLawJ2
    reference_flow_rate = 0.0001
    flow_rate_exponent = 50.0
    flow_rate_tol = 1
    strength_prop_name = flowstress1
  [../]
  [./flowrate2]
    type = HEVPFlowRatePowerLawJ2
    reference_flow_rate = 0.0001
    flow_rate_exponent = 50.0
    flow_rate_tol = 1
    strength_prop_name = flowstress2
  [../]
  [./ep_eqv1]
     type = HEVPEqvPlasticStrain
     intvar_rate_prop_name = ep_eqv_rate1
  [../]
  [./ep_eqv_rate1]
     type = HEVPEqvPlasticStrainRate
     flow_rate_prop_name = flowrate1
  [../]
  [./ep_eqv2]
     type = HEVPEqvPlasticStrain
     intvar_rate_prop_name = ep_eqv_rate2
  [../]
  [./ep_eqv_rate2]
     type = HEVPEqvPlasticStrainRate
     flow_rate_prop_name = flowrate2
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
    flow_rate_user_objects = 'flowrate1 flowrate2'
    strength_user_objects = 'flowstress1 flowstress2'
    internal_var_user_objects = 'ep_eqv1 ep_eqv2'
    internal_var_rate_user_objects = 'ep_eqv_rate1 ep_eqv_rate2'
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
[]
