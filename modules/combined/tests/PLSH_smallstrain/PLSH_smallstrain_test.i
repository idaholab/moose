#This is a test of the piece-wise linear strain hardening model using the small strain formulation.
#The exact same problem was run in Abaqus with exactly the same result.

[Mesh]
  file = 1x1x1cube.e

#  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  active = 'disp_x disp_y disp_z'

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

#active = ' '

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./plastic_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Functions]
  active = 'top_pull'

  [./top_pull]
    type = ParsedFunction
    value = t*(0.0625)
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

#active = ' '

  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]

  [./plastic_strain_xx]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_xx
    index = 0
  [../]

  [./plastic_strain_yy]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_yy
    index = 1
  [../]

  [./plastic_strain_zz]
    type = MaterialTensorAux
    tensor = plastic_strain
    variable = plastic_strain_zz
    index = 2
  [../]

 []


[BCs]
  active = 'y_pull_function  y_bot x_bot z_bot'

  [./y_pull_function]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = top_pull
  [../]

  [./x_bot]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]

  [./y_bot]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./z_bot]
    type = DirichletBC
    variable = disp_z
    boundary = 2
    value = 0.0
  [../]

[]

[Materials]
  [./vermont]
    type = PLSHPlasticMaterial
    formulation = lINeaR
    block = 1
    youngs_modulus = 2.1e5
    poissons_ratio = .3
    yield_stress = 50.0
    hardening_constant = 2000.
    tolerance = 1.e-5
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator -ksp_monitor -snes_ksp_ew'
  petsc_options_iname = '-snes_type -snes_ls -ksp_gmres_restart'
  petsc_options_value = 'ls         basic    101'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9

  start_time = 0.0
  end_time = 0.075
#  num_steps = 20
  dt = 0.00125
[]



[Output]
  file_base = out
  interval = 1
  output_initial = true
  exodus = true
  perf_log = true
[]


