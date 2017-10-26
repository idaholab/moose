#
# Cavity Pressure Test
#
# This test is designed to compute a negative number of moles
# to trigger an error check in the CavityPressureUserObject.
# The negative number of moles is achieved by supplying an
# open volume to the InternalVolume postprocessor, which
# calculates a negative volume.

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 2
[]

[Functions]
  [./temperature]
    type = PiecewiseLinear
    x = '0 1'
    y = '1 2'
    scale_factor = 100
  [../]
[]

[Variables]
  [./temperature]
    initial_condition = 100
  [../]
[]

[Modules/TensorMechanics/Master]
  [./block]
    strain = FINITE
    add_variables = true
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temperature
  [../]
[]

[BCs]
  [./no_x]
    type = PresetBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./no_y]
    type = PresetBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./temperatureInterior]
    type = FunctionDirichletBC
    boundary = 2
    function = temperature
    variable = temperature
  [../]
  [./CavityPressure]
    [./pressure]
      boundary = 'top bottom right'
      initial_pressure = 10e5
      R = 8.3143
      output_initial_moles = initial_moles
      temperature = aveTempInterior
      volume = internalVolume
      startup_time = 0.5
      output = ppress
    [../]
  [../]
[]

[Materials]
  [./elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stress1]
    type = ComputeFiniteStrainElasticStress
  [../]
  [./heatconduction]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    specific_heat = 1.0
  [../]
  [./density]
    type = Density
    density = 1.0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 0.5
  end_time = 1.0
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 'top bottom right'
    execute_on = 'initial linear'
  [../]
  [./aveTempInterior]
    type = AxisymmetricCenterlineAverageValue
    boundary = left
    variable = temperature
    execute_on = 'initial linear'
  [../]
[]

[Outputs]
  exodus = false
[]
