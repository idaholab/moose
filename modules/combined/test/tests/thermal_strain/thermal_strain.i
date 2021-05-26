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

[Modules/TensorMechanics/Master]
  add_variables = true
  strain = SMALL
  incremental = true
  generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  temperature = temp

  [./block1]
    eigenstrain_names = eigenstrain1
    block = 1
  [../]
  [./block2]
    eigenstrain_names = eigenstrain2
    block = 2
  [../]
  [./block3]
    eigenstrain_names = eigenstrain3
    block = 3
  [../]
  [./block4]
    eigenstrain_names = eigenstrain4
    block = 4
  [../]
  [./block5]
    eigenstrain_names = eigenstrain5
    block = 5
  [../]
  [./block6]
    eigenstrain_names = eigenstrain6
    block = 6
  [../]
  [./block7]
    eigenstrain_names = eigenstrain7
    block = 7
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
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
  [./thermal_strain1]
    type = ComputeThermalExpansionEigenstrain
    block = 1
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain1
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
  [./thermal_strain2]
    type = ComputeThermalExpansionEigenstrain
    block = 2
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain2
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
  [./thermal_strain3]
    type = ComputeThermalExpansionEigenstrain
    block = 3
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain3
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
  [./thermal_strain4]
    type = ComputeThermalExpansionEigenstrain
    block = 4
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain4
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
  [./thermal_strain5]
    type = ComputeThermalExpansionEigenstrain
    block = 5
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain5
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
  [./thermal_strain6]
    type = ComputeThermalExpansionEigenstrain
    block = 6
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain6
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
  [./thermal_strain7]
    type = ComputeThermalExpansionEigenstrain
    block = 7
    temperature = temp
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain7
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
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 0.5
  num_steps = 2
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
