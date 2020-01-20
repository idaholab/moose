#  Element mass tests

#  This series of tests is designed to compute the mass of elements based on
#  an evolving mass density calculation.  The tests consist of expansion and compression
#  of the elastic patch test model along each axis, uniform expansion and compression,
#  and shear in each direction.  The expansion and compression tests change the volume of
#  the elements.  The corresponding change in density should compensate for this so the
#  mass remains constant.  The shear tests should not result in a volume change, and this
#  is checked too.  The mass calculation is done with the post processor called Mass.

#  The tests/file names are as follows:

#  Expansion and compression along a single axis
#  expand_compress_x_test_out.e
#  expand_compress_y_test_out.e
#  expand_compress_z_test_out.e

#  Volumetric expansion and compression
#  uniform_expand_compress_test.i

#  Zero volume change shear along each axis
#  shear_x_test_out.e
#  shear_y_test_out.e
#  shear_z_test_out.e

#  The resulting mass calculation for these tests should always be = 1.

# This test is a duplicate of the uniform_expand_compress_test.i test for solid mechanics, and the
#   output of this tensor mechanics test is compared to the original
#   solid mechanics output.  The duplication is necessary to test the
#   migrated tensor mechanics version while maintaining tests for solid mechanics.

[Mesh]
  file = elastic_patch.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./rampConstant2]
    type = PiecewiseLinear
    x = '0.00 1.00  2.0   3.00'
    y = '0.00 0.25  0.0  -0.25'
    scale_factor = 1
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

[Kernels]
  [./TensorMechanics]
  [../]
[]

[BCs]
  [./9_y]
    type = DirichletBC
    variable = disp_y
    boundary = 9
    value = 0
  [../]
  [./10_y]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0
  [../]
  [./14_y]
    type = DirichletBC
    variable = disp_z
    boundary = 14
    value = 0
  [../]

  [./top]
    type = FunctionDirichletBC
    variable = disp_y
    preset = false
    boundary = 11
    function = rampConstant2
  [../]
  [./front]
    type = FunctionDirichletBC
    variable = disp_z
    preset = false
    boundary = 13
    function = rampConstant2
  [../]
  [./side]
    type = FunctionDirichletBC
    variable = disp_x
    preset = false
    boundary = 12
    function = rampConstant2
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = '1 2 3 4 5 6 7'
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]

  [./small_strain]
    type = ComputeSmallStrain
    block = ' 1 2 3 4 5 6 7'
  [../]

  [./elastic_stress]
    type = ComputeLinearElasticStress
    block = '1 2 3 4 5 6 7'
  [../]
[]

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 3
  end_time =3.0
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
    file_base = uniform_expand_compress_test_out
  [../]
[]

[Postprocessors]
  [./Mass]
    type = Mass
    variable = disp_x
    execute_on = 'initial timestep_end'
  [../]
[]
