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
# The mesh is composed of one block (2) with an interior cavity of volume 8.
#   Block 1 sits in the cavity and has a volume of 1.  Thus, the total
#   initial volume is 7.
# The test adjusts T in the following way:
#   T => T0 + beta * t
# with
#   beta = T0
#   T0 = 240.54443866068704
#   V0 = 7
#   n0 = f(p0)
#   p0 = 100
#   R = 8.314472 J * K^(-1) * mol^(-1)
#
# So, n0 = p0 * V0 / R / T0 = 100 * 7 / 8.314472 / 240.544439
#        = 0.35
#
# At t = 1, p = 200.

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Mesh]
  file = rz.e
[]

[Functions]
  [./temperature]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 2'
    scale_factor = 240.54443866068704
  [../]
[]

[Variables]
  [./disp_r]
  [../]
  [./disp_z]
  [../]
  [./temp]
    initial_condition = 240.54443866068704
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
    use_automatic_differentiation = true
  [../]
  [./heat]
    type = ADDiffusion
    variable = temp
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_r
    boundary = '1 2'
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_z
    boundary = '1 2'
    value = 0.0
  [../]
  [./temperatureInterior]
    type = ADFunctionDirichletBC
    preset = false
    boundary = 2
    function = temperature
    variable = temp
  [../]
  [./CavityPressure]
    [./1]
      boundary = 2
      initial_pressure = 100
      R = 8.314472
      temperature = aveTempInterior
      volume = internalVolume
      startup_time = 0.5
      output = ppress
      use_automatic_differentiation = true
    [../]
  [../]
[]

[Materials]
  [./elastic_tensor1]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = 1
  [../]
  [./strain1]
    type = ADComputeAxisymmetricRZFiniteStrain
    block = 1
  [../]
  [./stress1]
    type = ADComputeFiniteStrainElasticStress
    block = 1
  [../]
  [./elastic_tensor2]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = 2
  [../]
  [./strain2]
    type = ADComputeAxisymmetricRZFiniteStrain
    block = 2
  [../]
  [./stress2]
    type = ADComputeFiniteStrainElasticStress
    block = 2
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_abs_tol = 1e-10

  l_max_its = 20

  dt = 0.5
  end_time = 1.0
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 2
    execute_on = 'initial linear'
  [../]
  [./aveTempInterior]
    type = SideAverageValue
    boundary = 2
    variable = temp
    execute_on = 'initial linear'
  [../]
[]

[Outputs]
  exodus = true
  [./checkpoint]
    type = Checkpoint
    num_files = 1
  [../]
[]
