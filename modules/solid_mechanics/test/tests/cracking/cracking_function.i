#
# Simple pull test for cracking.
#
#
[Mesh]
   file = plate.e
   displacements = 'disp_x disp_y'
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
  [./crack_yy_flags]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0.001 0 -0.001 0'
  [../]
  [./pressure]
    type = PiecewiseLinear
    x = '0 0.4'
    y = '1.0e5 1.0e5'
  [../]
  [./fstress]
    type = ParsedFunction
    value = 'if(x > 0.667, 1.1e6, 0) + if(x<=0.667, 1.2e6, 0)'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
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
  [./crack_yy_flags]
    type = MaterialRealVectorValueAux
    property = crack_flags
    variable = crack_yy_flags
    component = 1
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
  [./pull]
    type = FunctionPresetBC
    variable = disp_x
    boundary = '3 4'
    function = displ
  [../]

  [./pin_x]
    type = PresetBC
    variable = disp_x
    boundary =  '1 2'
    value = 0
  [../]
  [./pin_y]
    type = PresetBC
    variable = disp_y
    boundary = '1 4'
    value = 0.0
  [../]
[]

[Materials]
  [./SolidModel]
    type = Elastic
    block = 1
    youngs_modulus = 200.0e7
    poissons_ratio = 0.0 #.3
    cracking_stress_function  = fstress
    cracking_stress = 1.2e6
    cracking_residual_stress = 0.0
    disp_x = disp_x
    disp_y = disp_y
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
  [./elem_crack_flags_x]
    type = ElementalVariableValue
    variable = crack_xx_flags
    elementid = 0
  [../]
  [./elem_crack_flags_y]
    type = ElementalVariableValue
    variable = crack_yy_flags
    elementid = 0
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK

  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101               '

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  l_tol = 1e-5
  start_time = 0.0
  end_time = 0.2
  dt = 0.0025
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  csv = true
[]
