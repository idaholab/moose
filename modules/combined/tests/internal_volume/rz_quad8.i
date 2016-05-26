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

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = meshes/rz_quad8.e
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 1e4
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

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]


[BCs]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  [../]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2'
    value = 0.0
  [../]

  [./Pressure]
    [./the_pressure]
      boundary = 3
      function = pressure
      disp_x = disp_x
      disp_y = disp_y
    [../]
  [../]
[]

[Materials]
  [./stiffStuff]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stiffStuff2]
    type = Elastic
    block = 2

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
[]

[Executioner]

  type = Transient

  solve_type = PJFNK



  nl_abs_tol = 1e-10

  l_max_its = 20

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
  exodus = true
  csv = true
[]
