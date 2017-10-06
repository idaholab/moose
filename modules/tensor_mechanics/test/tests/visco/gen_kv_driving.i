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
    type = RankTwoAux
    variable = creep_strain_xx
    rank_two_tensor = creep_strain
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
  [./eigen]
    type = ComputeEigenstrain
    eigenstrain_name = eigen_true
    eigen_base = '1e-3 1e-3 1e-3 0 0 0'
  [../]
  [./kelvin_voigt]
    type = GeneralizedKelvinVoigtModel
    creep_modulus = '10e9 10e9'
    creep_viscosity = '1 10'
    poisson_ratio = 0.2
    young_modulus = 10e9
    driving_eigenstrain = eigen_true
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'creep'
  [../]
  [./creep]
    type = LinearViscoelasticStressUpdate
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = 'eigen_true'
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

  l_max_its  = 50
  l_tol      = 1e-8
  nl_max_its = 20
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dtmin = 0.01
  end_time = 100
  [./TimeStepper]
    type = LogConstantDT
    first_dt = 0.1
    log_dt = 0.1
  [../]

[]

[Outputs]
  file_base = gen_kv_driving_out
  exodus = true
[]
