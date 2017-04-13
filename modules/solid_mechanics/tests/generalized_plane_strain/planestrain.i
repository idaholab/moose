[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  file = square.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./temp]
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]

  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
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

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./aux_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Postprocessors]
  [./react_z]
    type = MaterialTensorIntegralSM
    tensor = stress
    index = 2
    use_displaced_mesh = true
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_x = saved_x
    save_in_disp_y = saved_y
    use_displaced_mesh = true
    temp = temp
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
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

  [./strain_xx]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xx
    index = 0
  [../]
  [./strain_xy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xy
    index = 3
  [../]
  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
  [../]
  [./strain_zz]
    type = MaterialTensorAux
    tensor = total_strain
    variable = aux_strain_zz
    index = 2
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    value = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottomx]
    type = PresetBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./linelast]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    thermal_expansion = 0.02
    stress_free_temperature = 0.5
    temp = temp
    formulation = PlaneStrain
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none


# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-4

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-5

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
  num_steps = 5000
[]

[Outputs]
  exodus = true
[]
