#
# Prescribed pressure of 1e4 leads to xx, yy, and zz stress of 1e4.
#

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]#Comment
  file = pressure_rspherical.e
  construct_side_list_from_node_list = true
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
    order = FIRST
    family = LAGRANGE
  [../]

[]

[AuxVariables]

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

[] # AuxVariables

[SolidMechanics]
  [./solid]
    disp_r = disp_x
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]
[]

[BCs]

  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1'
    value = 0.0
  [../]

  [./Pressure]
    [./Pressure1]
      boundary = 2
      function = pressure
      disp_x = disp_x
    [../]
  [../]

[]

[Materials]
  [./stiffStuff]
    type = Elastic
    block = '1 2 3'

    disp_r = disp_x

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
[]

[Outputs]
  exodus = true
[]
