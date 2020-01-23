#
# Cavity Pressure Test
#
# This test is designed to compute an internal pressure based on
#   p = n * R * T / V
# where
#   p is the pressure
#   n is the amount of material in the volume (moles)
#   R is the universal gas constant
#   T is the temperature
#   V is the volume
#
# The mesh is composed of one block (1) with an interior cavity of volume 8.
#   Block 2 sits in the cavity and has a volume of 1.  Thus, the total
#   initial volume is 7.
# The test adjusts n, T, and V in the following way:
#   n => n0 + alpha * t
#   T => T0 + beta * t
#   V => V0 + gamma * t
# with
#   alpha = n0
#   beta = T0 / 2
#   gamma = - (0.003322259...) * V0
#   T0 = 240.54443866068704
#   V0 = 7
#   n0 = f(p0)
#   p0 = 100
#   R = 8.314472 J * K^(-1) * mol^(-1)
#
# So, n0 = p0 * V0 / R / T0 = 100 * 7 / 8.314472 / 240.544439
#        = 0.35
#
# The parameters combined at t = 1 gives p = 301.
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = 3d.e
[]

[Functions]
  [./displ_positive]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 0.0029069767441859684'
  [../]
  [./displ_negative]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 -0.0029069767441859684'
  [../]
  [./temp1]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 1.5'
    scale_factor = 240.54443866068704
  [../]
  [./material_input_function]
    type = PiecewiseLinear
    x = '0    1'
    y = '0 0.35'
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
    initial_condition = 240.54443866068704
  [../]
  [./material_input]
  [../]
[]

[AuxVariables]
  [./pressure_residual_x]
  [../]
  [./pressure_residual_y]
  [../]
  [./pressure_residual_z]
  [../]
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
    type = Diffusion
    variable = temp
    use_displaced_mesh = true
  [../]
  [./material_input_dummy]
    type = Diffusion
    variable = material_input
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress_xx
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress_yy
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_zz
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = stress_xy
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = stress_yz
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 0
    variable = stress_zx
  [../]
[]

[BCs]
  [./no_x_exterior]
    type = DirichletBC
    variable = disp_x
    boundary = '7 8'
    value = 0.0
  [../]
  [./no_y_exterior]
    type = DirichletBC
    variable = disp_y
    boundary = '9 10'
    value = 0.0
  [../]
  [./no_z_exterior]
    type = DirichletBC
    variable = disp_z
    boundary = '11 12'
    value = 0.0
  [../]
  [./prescribed_left]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 13
    function = displ_positive
  [../]
  [./prescribed_right]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 14
    function = displ_negative
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = '15 16'
    value = 0.0
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = '17 18'
    value = 0.0
  [../]
  [./no_x_interior]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  [../]
  [./no_y_interior]
    type = DirichletBC
    variable = disp_y
    boundary = '3 4'
    value = 0.0
  [../]
  [./no_z_interior]
    type = DirichletBC
    variable = disp_z
    boundary = '5 6'
    value = 0.0
  [../]
  [./temperatureInterior]
    type = FunctionDirichletBC
    boundary = 100
    function = temp1
    variable = temp
  [../]
  [./MaterialInput]
    type = FunctionDirichletBC
    boundary = '100 13 14 15 16'
    function = material_input_function
    variable = material_input
  [../]
  [./CavityPressure]
    [./1]
      boundary = 100
      initial_pressure = 100
      material_input = materialInput
      R = 8.314472
      temperature = aveTempInterior
      volume = internalVolume
      startup_time = 0.5
      output = ppress
      save_in = 'pressure_residual_x pressure_residual_y pressure_residual_z'
    [../]
  [../]
[]

[Materials]
  [./elast_tensor1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e1
    poissons_ratio = 0
    block = 1
  [../]
  [./strain1]
    type = ComputeFiniteStrain
    block = 1
  [../]
  [./stress1]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]
  [./elast_tensor2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0
    block = 2
  [../]
  [./strain2]
    type = ComputeFiniteStrain
    block = 2
  [../]
  [./stress2]
    type = ComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Executioner]
  type = Transient

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_rel_tol = 1e-12
  l_tol = 1e-12

  l_max_its = 20

  dt = 0.5
  end_time = 1.0
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 100
    execute_on = 'initial linear'
  [../]
  [./aveTempInterior]
    type = SideAverageValue
    boundary = 100
    variable = temp
    execute_on = 'initial linear'
  [../]
  [./materialInput]
    type = SideAverageValue
    boundary = '7 8 9 10 11 12'
    variable = material_input
    execute_on = linear
  [../]
[]

[Outputs]
  exodus = true
[]
