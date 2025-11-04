#
# Internal Volume Test
#
# This test is designed to compute the internal volume of a space considering
#   an embedded volume inside.
#
# The mesh is composed of one block (1) with an interior cavity of volume 8.
#   Block 2 sits in the cavity and has a volume of 1.  Thus, the total volume
#   is 7.
#
# The internal volume is then adjusted by a piecewise linear time varying
# function.  Thus, the total volume is 7 plus the addition at the particular
# time.
#
#  Time |  Addition  | Total volume
#   0   |    0.0     |     7.0
#   1   |    3.0     |    10.0
#   2   |    7.0     |    14.0
#   3   |   -3.0     |     4.0
#
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = meshes/hex8.e
[]

[Functions]
  [./step]
    type = PiecewiseLinear
    x = '0. 1. 2. 3.'
    y = '0. 0. 1e-2 0.'
    scale_factor = 0.5
  [../]
  [./addition]
    type = PiecewiseLinear
    x = '0. 1. 2. 3.'
    y = '0. 3. 7. -3.'
  [../]
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

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    volumetric_locking_correction = true
    incremental = true
    strain = FINITE
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 100
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]

  [./prescribed_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 100
    function = step
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  start_time = 0.0
  dt = 1.0
  end_time = 3.0
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 100
    addition = addition
    execute_on = 'initial timestep_end'
  [../]

  [./dispZ]
    type = ElementAverageValue
    block = '1 2'
    variable = disp_z
  [../]
[]

[Outputs]
  csv = true
[]
