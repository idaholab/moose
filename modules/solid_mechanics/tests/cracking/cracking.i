#
# Simple pull test for cracking.
# The stress increases for two steps and then drops to zero.
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
  [./displ]
    type = PiecewiseLinear
    x = '0 1 2 3  4'
    y = '0 1 0 -1 0'
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
  [./pull]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = displ
  [../]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = 3
    value = 0.0
  [../]
[]

[Materials]
  [./fred]
    type = Elastic
    block = 1
    youngs_modulus = 2.8e7
    poissons_ratio = 0 #.3
    cracking_stress = 1.68e6

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
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
  end_time = 0.1
  dt = 0.025
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
