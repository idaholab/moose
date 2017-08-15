#
# Test to exercise the exponential stress release
#
# Stress vs. strain should show a linear relationship until cracking,
#   an exponential stress release, a linear relationship back to zero
#   strain, a linear relationship with the original stiffness in
#   compression and then back to zero strain, a linear relationship
#   back to the exponential curve, and finally further exponential
#   stress release.
#

[Mesh]
  file = cracking_test.e
  displacements = 'disp_x disp_y disp_z'
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

[AuxVariables]

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

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

[]

[Functions]
  [./displx]
    type = PiecewiseLinear
#   x = '0 1'
#   y = '0 .0035'
    x = '0 1       2  3      4 5       6'
    y = '0 0.00175 0 -0.0001 0 0.00175 0.0035'
  [../]
  [./disply]
    type = PiecewiseLinear
    x = '0 5 6'
    y = '0 0 .00175'
  [../]
  [./displz]
    type = PiecewiseLinear
    x = '0 2 3'
    y = '0 0 .0035'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]


[AuxKernels]
  [./strain_xx]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 0
  [../]

  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]

  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]

  [./stress_zz]
    type = MaterialTensorAux
    variable = stress_zz
    tensor = stress
    index = 2
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    variable = stress_xy
    tensor = stress
    index = 3
  [../]

  [./stress_yz]
    type = MaterialTensorAux
    variable = stress_yz
    tensor = stress
    index = 4
  [../]

  [./stress_zx]
    type = MaterialTensorAux
    variable = stress_zx
    tensor = stress
    index = 5
  [../]
[]


[BCs]
  [./pullx]
    type = FunctionPresetBC
    #type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = displx
  [../]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = '11 12'
    value = 0.0
  [../]

  [./move_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = '15 16'
    function = disply
  [../]

  [./back]
    type = PresetBC
    variable = disp_z
    boundary = '3'
    value = 0.0
  [../]
[]

[Materials]
  [./fred]
    type = Elastic
    block = 1
    youngs_modulus = 186.5e9
    poissons_ratio = .316
    cracking_stress = 119.3e6
    cracking_release = exponential

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    formulation = linear
  [../]
[]

[Postprocessors]
  [./strain_xx]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_xx
  [../]
  [./stress_xx]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_xx
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'

  line_search = 'none'
  l_max_its = 100
  l_tol = 1e-6

  nl_max_its = 10
  nl_rel_tol = 1e-9

  nl_abs_tol = 5.e-8

  start_time = 0.0
  dt = 0.02
  dtmin = 0.02
  num_steps = 300
[]

[Outputs]
  exodus = true
  csv = true
[]

