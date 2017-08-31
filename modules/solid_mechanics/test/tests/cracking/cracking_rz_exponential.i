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
  file = cracking_rz_test.e
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Problem]
  coord_type = RZ
[]

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
    scaling = 1e-3
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

[]

[AuxVariables]

  [./strain_yy]
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
  [./disply]
    type = PiecewiseLinear
    x = '0 1       2  3      4 5       6'
    y = '0 0.00175 0 -0.0001 0 0.00175 0.0035'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]


[AuxKernels]
  [./strain_yy]
    type = MaterialTensorAux
    variable = strain_yy
    tensor = total_strain
    index = 1
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
  [./pully]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 4
    function = disply
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./sides]
    type = PresetBC
    variable = disp_x
    boundary = 1
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

    disp_r = disp_x
    disp_z = disp_y
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
  nl_rel_tol = 1e-10

  nl_abs_tol = 6e-10

  start_time = 0.0
  end_time = 6.0
  dt = 0.005
  dtmin = 0.005
[]

[Postprocessors]
  [./stress_yy]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yy
  [../]

  [./strain_yy]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_yy
  [../]
[]

[Outputs]
  exodus = true
  csv = true
[]

