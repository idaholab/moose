# Simple 3D test

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 1
    nz = 1
  []
[]

[AuxVariables]
  [strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []

  [strain_xx]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [strain_yy]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [strain_zz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [strain_xy]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [strain_xz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [strain_yz]
    type = RankTwoAux
    rank_two_tensor = mechanical_strain
    variable = strain_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []
[]

[Kernels]
  [sdx]
    type = UpdatedLagrangianStressDivergence
    variable = disp_x
    component = 0
    use_displaced_mesh = false
  []
  [sdy]
    type = UpdatedLagrangianStressDivergence
    variable = disp_y
    component = 1
    use_displaced_mesh = false
  []
  [sdz]
    type = UpdatedLagrangianStressDivergence
    variable = disp_z
    component = 2
    use_displaced_mesh = false
  []
[]

[Functions]
  [pullx]
    type = ParsedFunction
    expression = 't'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [lefty]
    type = DirichletBC
    preset = true
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [leftz]
    type = DirichletBC
    preset = true
    boundary = back
    variable = disp_z
    value = 0.0
  []
  [pull_x]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = pullx
  []
[]

[UserObjects]
  [./str]
    type = TensorMechanicsHardeningPowerRule
    value_0 = 100.0
    epsilon0 = 0.1
    exponent = 2.0
  [../]
  [./j2]
    type = TensorMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianWrappedStress
  []
  [compute_stress_base]
    type = ComputeMultiPlasticityStress
    plastic_models = j2
    ep_plastic_tolerance = 1E-9
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [./strain]
    type = ElementAverageValue
    variable = strain_xx
  []
  [./stress]
    type = ElementAverageValue
    variable = stress_xx
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.001
  dtmin = 0.001
  end_time = 0.05
[]

[Outputs]
  exodus = false
  csv = true
[]
