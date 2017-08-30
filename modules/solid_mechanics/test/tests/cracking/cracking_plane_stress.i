################################################################################
#
# 1x1x1 cube, single element
# simulate plane stress
# pull in +y direction on right surface to produce shear strain
#
#
#
#          ____________
#         /|          /|
#        / |  5      / |                       -X  Left   1
#       /__________ /  |                       +X  Right  4
#      |   |    3  |   |                       +Y  Top    5
#      | 1 |       | 4 |                       -Y  Bottom 2
#      |   |_6_____|___|           y           +Z  Front  6
#      |  /        |  /            ^           -Z  Back   3
#      | /    2    | /             |
#      |/__________|/              |
#                                  ----> x
#                                 /
#                                /
#                               z
#
#
#
#################################################################################

[Mesh]
  file = cube.e
  displacements = 'disp_x disp_y disp_z'
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
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

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_yz]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_zx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_xx_flags]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0.0026 0 -0.0026 0'
  [../]
  [./pressure]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0   0    0   0'
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
  [./crack_xx_flags]
    type = MaterialRealVectorValueAux
    property = crack_flags
    variable = crack_xx_flags
    component = 0
    block = 1
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

  [./strain_xx]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 0
  [../]

  [./strain_yy]
    type = MaterialTensorAux
    variable = strain_yy
    tensor = total_strain
    index = 1
  [../]

  [./strain_zz]
    type = MaterialTensorAux
    variable = strain_zz
    tensor = total_strain
    index = 2
  [../]

  [./strain_xy]
    type = MaterialTensorAux
    variable = strain_xy
    tensor = total_strain
    index = 3
  [../]

  [./strain_yz]
    type = MaterialTensorAux
    variable = strain_yz
    tensor = total_strain
    index = 4
  [../]

  [./strain_zx]
    type = MaterialTensorAux
    variable = strain_zx
    tensor = total_strain
    index = 5
  [../]

[]


[BCs]
  [./pull_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 4
    function = displ
  [../]
  [./pin_x]
    type = PresetBC
    variable = disp_x
    boundary = '1  4'
    value = 0.0
  [../]
  [./pin_y]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = '3'
    value = 0.0
  [../]
  [./front]
    type = Pressure
    variable = disp_z
    component = 2
    boundary = 6
    function = pressure
    factor   = 1.0
  [../]
[]

[Materials]
  [./fred]
    type = Elastic
    block = 1
    youngs_modulus = 200.0e3
    poissons_ratio = 0.3
    cracking_stress = 120
    cracking_release = exponential
    cracking_residual_stress = 0.1
    cracking_beta = 0.1
    compute_method = ShearRetention
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]

[]
[Postprocessors]
  [./elem_stress_xx]
    type = ElementalVariableValue
    variable = stress_xx
    elementid = 0
  [../]
  [./elem_strain_xx]
    type = ElementalVariableValue
    variable = strain_xx
    elementid = 0
  [../]
  [./elem_stress_yy]
    type = ElementalVariableValue
    variable = stress_yy
    elementid = 0
  [../]
  [./elem_strain_yy]
    type = ElementalVariableValue
    variable = strain_yy
    elementid = 0
  [../]
  [./elem_stress_zz]
    type = ElementalVariableValue
    variable = stress_zz
    elementid = 0
  [../]
  [./elem_strain_zz]
    type = ElementalVariableValue
    variable = strain_zz
    elementid = 0
  [../]
  [./elem_stress_xy]
    type = ElementalVariableValue
    variable = stress_xy
    elementid = 0
  [../]
  [./elem_strain_xy]
    type = ElementalVariableValue
    variable = strain_xy
    elementid = 0
  [../]
  [./elem_stress_yz]
    type = ElementalVariableValue
    variable = stress_yz
    elementid = 0
  [../]
  [./elem_strain_yz]
    type = ElementalVariableValue
    variable = strain_yz
    elementid = 0
  [../]
  [./elem_stress_zx]
    type = ElementalVariableValue
    variable = stress_yz
    elementid = 0
  [../]
  [./elem_strain_zx]
    type = ElementalVariableValue
    variable = strain_yz
    elementid = 0
  [../]
  [./elem_crack_flags]
    type = ElementalVariableValue
    variable = crack_xx_flags
    elementid = 0
  [../]
[]
[Executioner]
  type = Transient
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-5
  start_time = 0.0
  end_time = 0.4
  dt = 0.04
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    perf_log = true
    output_linear = true
  [../]
 csv = true
[]
