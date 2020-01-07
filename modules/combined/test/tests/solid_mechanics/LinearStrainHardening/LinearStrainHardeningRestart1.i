#

[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = LinearStrainHardening_test.e
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

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_mag]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Functions]
  [./top_pull]
    type = ParsedFunction
    value = t/5.0
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]


[AuxKernels]

  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]

  [./plastic_strain_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]

  [./plastic_strain_yy]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  [../]

  [./plastic_strain_zz]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  [../]

  [./plastic_strain_mag]
    type = MaterialRealAux
    property = effective_plastic_strain
    variable = plastic_strain_mag
  [../]

 []


[BCs]

  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = top_pull
  [../]

  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

[]


[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = '1'
  [../]
  [./stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'isoplas'
    block = '1'
  [../]
  [./isoplas]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 2.4e2
    hardening_constant = 1206
    relative_tolerance = 1e-25
    absolute_tolerance = 1e-5
  [../]
[]



[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'


  line_search = 'none'


  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  end_time = 0.0105
  num_steps = 4
  dt = 1.5e-3
[]



[Outputs]
  exodus = true
  csv = true
  [./out]
    type = Checkpoint
    num_files = 1
  [../]
[]
