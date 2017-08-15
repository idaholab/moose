#
# Rotation Test
#
# This test is designed to compute a uniaxial stress and then follow that
#   stress as the mesh is rotated 90 degrees.
#
# The mesh is composed of one block with a single element.  The nodal
#   displacements in the x and y directions are prescribed.  Poisson's
#   ratio is zero.
#

[Mesh]#Comment
  file = rotation_test.e
  displacements = 'disp_x disp_y disp_z'
[] # Mesh

[Functions]
  [./x_200]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2*(t-t0)) - 1.0)'
  [../]
  [./y_200]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, (1.0+delta)*sin(pi/2*(t-t0)))'
  [../]

  [./x_300]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2.0*(t-t0)) - sin(pi/2.0*(t-t0)) - 1.0)'
  [../]
  [./y_300]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) + (1+delta)*sin(pi/2.0*(t-t0)) - 1.0)'
  [../]

  [./x_400]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, -sin(pi/2.0*(t-t0)))'
  [../]
  [./y_400]
    type = ParsedFunction
    vars = 'delta t0'
    vals = '-1e-6 1.0'
    value = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) - 1.0)'
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
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
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
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
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

  [./x_200]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 200
    function = x_200
  [../]
  [./y_200]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 200
    function = y_200
  [../]

  [./x_300]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 300
    function = x_300
  [../]
  [./y_300]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 300
    function = y_300
  [../]

  [./x_400]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 400
    function = x_400
  [../]
  [./y_400]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 400
    function = y_400
  [../]

  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = '100 200 300 400'
    value = 0.0
  [../]

[] # BCs

[Materials]
  [./test]
    type = Elastic
    block = 1

    poissons_ratio = 0
    shear_modulus = 5e6

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]

[] # Materials

[Executioner]

  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm       lu'

  nl_rel_tol = 1e-30

  l_max_its = 20

  start_time = 0.0
  dt = 0.01
  end_time = 2.0
[] # Executioner


[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[] # Outputs
