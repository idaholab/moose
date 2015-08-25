#
# Pressure Test
#
# This test is taken from the Abaqus verification manual:
#   "1.3.4 Axisymmetric solid elements"
#
# The two lower nodes are not allowed to translate in the z direction.
# Step 1:
#   Pressure of 1000 is applied on each face.
# Step 2:
#   Step 1 load plus a pressure on the vertical faces that varies from
#   0 to 1000 from top to bottom.
#
# Solution:
# Step 1:
#    Stress xx, yy, zz = -1000
#    Stress xy = 0
# Step 2:
#    Stress xx, zz = -1500
#    Stress yy = -1000
#    Stress xy = 0

[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
[]

[Problem]
  coord_type = RZ
[]

[Mesh]#Comment
  file = pressure_rz_test.e
[] # Mesh

[Functions]
  [./constant]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 1e3
  [../]
  [./vary]
    type = ParsedFunction
    value = 'if(t <= 1, 1000 , 1000+1000*(1-y))'
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

[] # Variables

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

[] # AuxVariables

[SolidMechanics]

  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]

[] # SolidMechanics

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
#  [./stress_xy]
#    type = MaterialTensorAux
#    tensor = stress
#    variable = stress_xy
#    index = 3
#  [../]
  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]
  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]

[] # AuxKernels

[BCs]

  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./Pressure]
    [./Pressure1]
      boundary = '3 4'
      function = constant
    [../]
    [./Pressure2]
      boundary = '1 2'
      function = vary
    [../]
  [../]

[] # BCs

[Materials]

  [./stiffStuff]
    type = Elastic
    block = 1

    disp_r = disp_x
    disp_z = disp_y

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
  nl_rel_tol = 1e-12

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
