# Patch Test

# This test is designed to compute displacements from a thermal strain.

# The cube is displaced by 1e-6 units in x, 2e-6 in y, and 3e-6 in z.
#  The faces are sheared as well (1e-6, 2e-6, and 3e-6 for xy, yz, and
#  zx).  This gives a uniform strain/stress state for all six unique
#  tensor components.

# The temperature moves 100 degrees, and the coefficient of thermal
#  expansion is 1e-6.  Therefore, the strain (and the displacement
#  since this is a unit cube) is 1e-4.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = thermal_strain_test.e
[]

[Functions]
  [./tempFunc]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '117.56 217.56'
  [../]
[]

[Variables]
  [./disp_x]
  [../]

  [./disp_y]
  [../]

  [./disp_z]
  [../]

  [./temp]
    initial_condition = 117.56
  [../]
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]

  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 9
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 14
    value = 0
  [../]

  [./temp]
    type = FunctionDirichletBC
    variable = temp
    boundary = '10 12'
    function = tempFunc
  [../]
[]

[Materials]
  [./elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    block = 1
    bulk_modulus = 0.333333333333e6
    poissons_ratio = 0.0
  [../]
  [./strain1]
    type = ComputeIncrementalSmallStrain
    block = 1
    eigenstrain_names = eigenstrain1
  [../]
  [./thermal_strain1]
    type = ComputeThermalExpansionEigenstrain
    block = 1
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain1
    incremental_form = true
  [../]
  [./stress1]
    type = ComputeStrainIncrementBasedStress
    block = 1
  [../]

  [./elasticity_tensor2]
    type = ComputeIsotropicElasticityTensor
    block = 2
    bulk_modulus = 0.333333333333e6
    lambda = 0.0
  [../]
  [./strain2]
    type = ComputeIncrementalSmallStrain
    block = 2
    eigenstrain_names = eigenstrain2
  [../]
  [./thermal_strain2]
    type = ComputeThermalExpansionEigenstrain
    block = 2
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain2
    incremental_form = true
  [../]
  [./stress2]
    type = ComputeStrainIncrementBasedStress
    block = 2
  [../]

  [./elasticity_tensor3]
    type = ComputeIsotropicElasticityTensor
    block = 3
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./strain3]
    type = ComputeIncrementalSmallStrain
    block = 3
    eigenstrain_names = eigenstrain3
  [../]
  [./thermal_strain3]
    type = ComputeThermalExpansionEigenstrain
    block = 3
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain3
    incremental_form = true
  [../]
  [./stress3]
    type = ComputeStrainIncrementBasedStress
    block = 3
  [../]

  [./elasticity_tensor4]
    type = ComputeIsotropicElasticityTensor
    block = 4
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./strain4]
    type = ComputeIncrementalSmallStrain
    block = 4
    eigenstrain_names = eigenstrain4
  [../]
  [./thermal_strain4]
    type = ComputeThermalExpansionEigenstrain
    block = 4
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain4
    incremental_form = true
  [../]
  [./stress4]
    type = ComputeStrainIncrementBasedStress
    block = 4
  [../]

  [./elasticity_tensor5]
    type = ComputeIsotropicElasticityTensor
    block = 5
    youngs_modulus = 1e6
    lambda = 0.0
  [../]
  [./strain5]
    type = ComputeIncrementalSmallStrain
    block = 5
    eigenstrain_names = eigenstrain5
  [../]
  [./thermal_strain5]
    type = ComputeThermalExpansionEigenstrain
    block = 5
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain5
    incremental_form = true
  [../]
  [./stress5]
    type = ComputeStrainIncrementBasedStress
    block = 5
  [../]

  [./elasticity_tensor6]
    type = ComputeIsotropicElasticityTensor
    block = 6
    youngs_modulus = 1e6
    shear_modulus = 5e5
  [../]
  [./strain6]
    type = ComputeIncrementalSmallStrain
    block = 6
    eigenstrain_names = eigenstrain6
  [../]
  [./thermal_strain6]
    type = ComputeThermalExpansionEigenstrain
    block = 6
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain6
    incremental_form = true
  [../]
  [./stress6]
    type = ComputeStrainIncrementBasedStress
    block = 6
  [../]

  [./elasticity_tensor7]
    type = ComputeIsotropicElasticityTensor
    block = 7
    shear_modulus = 5e5
    poissons_ratio = 0.0
  [../]
  [./strain7]
    type = ComputeIncrementalSmallStrain
    block = 7
    eigenstrain_names = eigenstrain7
  [../]
  [./thermal_strain7]
    type = ComputeThermalExpansionEigenstrain
    block = 7
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain7
    incremental_form = true
  [../]
  [./stress7]
    type = ComputeStrainIncrementBasedStress
    block = 7
  [../]

  [./heat]
    type = HeatConductionMaterial
    block = '1 2 3 4 5 6 7'
    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = '1 2 3 4 5 6 7'
    density = 1.0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 0.5
  num_steps = 2
  end_time = 1.0
[]

[Outputs]
  file_base = thermal_strain_test_out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
