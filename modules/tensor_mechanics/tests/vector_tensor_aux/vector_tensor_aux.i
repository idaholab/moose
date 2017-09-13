[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX8
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscous_strain_1_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscous_strain_2_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./viscous_strain_3_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./spring_1_xxxx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./spring_2_xxyy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./spring_3_xyxy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_strain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./viscous_strain_1]
    type = VectorRankTwoAux
    variable = viscous_strain_1_xx
    rank_two_tensors = viscous_strains
    position = 0
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./viscous_strain_2]
    type = VectorRankTwoAux
    variable = viscous_strain_2_xx
    rank_two_tensors = viscous_strains
    position = 1
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./viscous_strain_3]
    type = VectorRankTwoAux
    variable = viscous_strain_3_xx
    rank_two_tensors = viscous_strains
    position = 2
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  [../]
  [./spring_1]
    type = VectorRankFourAux
    variable = spring_1_xxxx
    rank_four_tensors = springs_elasticity_tensors
    position = 0
    index_i = 0
    index_j = 0
    index_k = 0
    index_l = 0
    execute_on = timestep_end
  [../]
  [./spring_2]
    type = VectorRankFourAux
    variable = spring_2_xxyy
    rank_four_tensors = springs_elasticity_tensors
    position = 1
    index_i = 0
    index_j = 0
    index_k = 1
    index_l = 1
    execute_on = timestep_end
  [../]
  [./spring_3]
    type = VectorRankFourAux
    variable = spring_3_xyxy
    rank_four_tensors = springs_elasticity_tensors
    position = 2
    index_i = 0
    index_j = 1
    index_k = 0
    index_l = 1
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./symmy]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./symmz]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0
  [../]
  [./axial_load]
    type = NeumannBC
    variable = disp_x
    boundary = right
    value    = 10e6
  [../]
[]

[Materials]
  [./kelvin_voigt]
    type = GeneralizedKelvinVoigtModel
    creep_modulus = '10e9 20e9 30e9'
    creep_viscosity = '0.1 1 10'
    poisson_ratio = 0.2
    young_modulus = 10e9
  [../]
  [./stress]
    type = ComputeLinearViscoelasticStress
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[UserObjects]
  [./update]
    type = LinearViscoelasticityManager
    viscoelastic_model = kelvin_voigt
  [../]
[]

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./viscous_strain_1_xx]
    type = ElementAverageValue
    variable = viscous_strain_1_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./viscous_strain_2_xx]
    type = ElementAverageValue
    variable = viscous_strain_2_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./viscous_strain_3_xx]
    type = ElementAverageValue
    variable = viscous_strain_3_xx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./spring_1_xxxx]
    type = ElementAverageValue
    variable = spring_1_xxxx
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./spring_2_xxyy]
    type = ElementAverageValue
    variable = spring_2_xxyy
    block = 'ANY_BLOCK_ID 0'
  [../]
  [./spring_3_xyxy]
    type = ElementAverageValue
    variable = spring_3_xyxy
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  l_max_its  = 50
  l_tol      = 1e-10
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  dtmin = 0.01
  end_time = 100
  [./TimeStepper]
    type = LogConstantDT
    first_dt = 0.1
    log_dt = 0.1
  [../]

[]

[Outputs]
  file_base = vector_tensor_aux_out
  exodus = true
[]
