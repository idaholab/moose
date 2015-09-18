#
# Pressure Test
#
# This test is designed to compute pressure loads on three faces of a unit cube.
#
# The mesh is composed of one block with a single element.  Symmetry bcs are
#   applied to the faces opposite the pressures.  Poisson's ratio is zero,
#   which makes it trivial to check displacements.
#

[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
[../]

[Mesh]#Comment
  file = pressure_test.e
  displacements = 'disp_x disp_y disp_z'
[] # Mesh

[Functions]
  [./rampConstant]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 1.0
  [../]
  [./zeroRamp]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 0. 1.'
    scale_factor = 2.0
  [../]
  [./rampUnramp]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 0.'
    scale_factor = 10.0
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

[] # Variables

[SolidMechanics]

  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]

[] # SolidMechanics

[BCs]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 5
    value = 0.0
  [../]

  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  [../]

  [./Pressure]
    [./Side1]
      boundary = 1
      function = rampConstant
    [../]
    [./Side2]
      boundary = 2
      function = zeroRamp
    [../]
    [./Side3]
      boundary = 3
      function = rampUnramp
    [../]
  [../]

[] # BCs

[Materials]

  [./stiffStuff]
    type = LinearIsotropicMaterial
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z

    youngs_modulus = 1e6
    poissons_ratio = 0.0
    thermal_expansion = 1e-5
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  nl_abs_tol = 1e-10

  l_max_its = 20

  start_time = 0.0
  dt = 1.0
  num_steps = 2
  end_time = 2.0
[] # Executioner

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
