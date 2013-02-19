# Testing the UMAT Interface - creep linear strain hardening model using the small strain formulation - visco-plastic material.  
# Note that this isn't a thermal or irradiation creep model.

[Mesh]
  file = 1x1x1cube.e
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
  # active = ' '
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./top_pull]
    type = ParsedFunction
    value = t/100
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
  # active = ' '
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
[]

[BCs]
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
  [./constant]
    type = AbaqusUmatMaterial
    formulation = linear
    block = 1
    youngs_modulus = 1000.
    poissons_ratio = .3
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    mechanical_constants = '1000. 0.3 10. 100.'
    plugin = ../../plugins/solid_mechanics/linear_strain_hardening
    num_state_vars = 3
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator -ksp_monitor -snes_ksp_ew'
  petsc_options_iname = '-snes_type -snes_ls -snes_linesearch_type -ksp_gmres_restart'
  petsc_options_value = 'ls         basic    basic                    101'
  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
  start_time = 0.0
  num_steps = 30
  dt = 1.
[]

[Output]
  file_base = out
  interval = 1
  output_initial = true
  elemental_as_nodal = true
  exodus = true
  perf_log = true
[]

