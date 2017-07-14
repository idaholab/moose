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
  [./creep_strain_xx]
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
  [./creep_strain_xx]
    type = VectorRankTwoAux
    variable = creep_strain_xx
    rank_two_tensors = viscous_strains
    position = 0
    index_j = 0
    index_i = 0
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
[]

[Materials]
  [./stress]
    type = ComputeLinearViscoelasticStress
  [../]
  [./eigen]
    type = ComputeEigenstrain
    eigenstrain_name = eigen_true
    eigen_base = '1e-3 1e-3 1e-3 0 0 0'
  [../]
  [./maxwell]
    type = GeneralizedMaxwellModel
    creep_modulus = '3.333333e9 3.333333e9'
    creep_viscosity = '1 10'
    poisson_ratio = 0.2
    young_modulus = 10e9
  [../]
  [./adjusted]
    type = ViscoelasticityAdjustedEigenstrain
    base_name = eigen_adjusted
    source_eigenstrain = eigen_true
    viscoelastic_model = maxwell
  [../]
  [./creep]
    type = ComputeLinearViscoelasticCreepStrain
    viscoelastic_model = maxwell
    driving_eigenstrain = eigen_adjusted_creep_strain
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = 'eigen_adjusted_creep_strain'
  [../]
[]

[UserObjects]
  [./visco_update]
    type = ViscoelasticityUpdate
    creep_strain_names = 'creep adjusted'
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
  [./creep_strain_xx]
    type = ElementAverageValue
    variable = creep_strain_xx
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
  l_tol      = 1e-8
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dtmin = 0.01
  end_time = 100
  [./TimeStepper]
    type = TimeSequenceStepper
    time_sequence  = '0 0.1 0.125 0.158  0.199 0.251 0.316 0.398 0.501 0.630 0.749 1 1.258 1.584 1.995 2.511 3.162 3.981 5.011 6.309 7.943 10 12.589 15.848 19.952 25.118 31.622 39.810 50.118 63.095 79.432 100'
  [../]

[]

[Outputs]
  file_base = gen_maxwell_adjusted_out
  exodus = true
[]
