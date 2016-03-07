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


[Mesh]#Comment
  file = elastic_patch.e
  displacements = 'disp_x disp_y disp_z'
[] # Mesh

[Functions]
  [./rampConstant1]
    type = PiecewiseLinear
    x = '0.00 1.00  2.0   3.00'
    y = '0.00 0.25  0.0  -0.25'
    scale_factor = 1
  [../]
[] # Functions

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

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[BCs]
  [./bot_x]
    type = DirichletBC
    variable = disp_x
    value = 0.0
  [../]
  [./bot_y]
    type = DirichletBC
    variable = disp_y
    value = 0
  [../]
  [./bot_z]
    type = DirichletBC
    variable = disp_z
    value = 0
  [../]
  [./top]
    type = FunctionDirichletBC
    function = rampConstant1
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff2]
    type = Elastic
    block = 2
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff3]
    type = Elastic
    block = 3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff4]
    type = Elastic
    block = 4
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff5]
    type = Elastic
    block = 5
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff6]
    type = Elastic
    block = 6
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
  [../]
  [./stiffStuff7]
    type = Elastic
    block = 7
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    youngs_modulus = 1e6
    poissons_ratio = 0.0
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
  dt = 1.0
  num_steps = 3
  end_time = 3.0
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]

[Postprocessors]
  [./Mass]
    type = Mass
    variable = disp_x
    execute_on = 'initial timestep_end'
  [../]
[]
