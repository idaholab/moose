#
# Volume Test
#
# This test is designed to compute the volume of a space when displacements
#   are imposed.
#
# The mesh is composed of one block (1) with two elements.  The mesh is
#   such that the initial volume is 1.  One element face is displaced to
#   produce a final volume of 2.
#
#     r1
#   +----+   -
#   |    |   |
#   +----+   h    V1 = pi * h * r1^2
#   |    |   |
#   +----+   -
#
#   becomes
#
#   +----+
#   |     \
#   +------+      v2 = pi * h/2 * ( r2^2 + 1/3 * ( r2^2 + r2*r1 + r1^2 ) )
#   |      |
#   +------+
#      r2
#
#   r1 = 1
#   r2 = 1.5380168369562588
#   h  = 1/pi
#
#  Note:  Because the InternalVolume PP computes cavity volumes as positive,
#         the volumes reported are negative.
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = meshes/rz_displaced_quad8.e
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./disp_x]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 0.5380168369562588'
  [../]
  [./disp_x2]
    type = PiecewiseLinear
    scale_factor = 0.5
    x = '0. 1.'
    y = '0. 0.5380168369562588'
  [../]
[]

[Variables]
  [./disp_x]
    order = SECOND
    family = LAGRANGE
  [../]

  [./disp_y]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    volumetric_locking_correction = false
    decomposition_method = EigenSolution
    incremental = true
    strain = FINITE
  [../]
[]

[BCs]
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]

  [./x]
    type = FunctionDirichletBC
    boundary = 3
    variable = disp_x
    function = disp_x
  [../]
  [./x2]
    type = FunctionDirichletBC
    boundary = 4
    variable = disp_x
    function = disp_x2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  start_time = 0.0
  dt = 1.0
  end_time = 1.0

  [./Quadrature]
    order = THIRD
  [../]
[]

[Postprocessors]
  [./internalVolume]
    type = InternalVolume
    boundary = 2
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
