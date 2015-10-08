#

[Mesh]
  file = necking_quad4.e
  displacements = 'disp_x disp_y'
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

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
#    save_in_disp_x = force_x
    save_in_disp_y = force_y
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
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
#  [./force_x]
#    order = FIRST
#    family = LAGRANGE
#  [../]
  [./force_y]
    order = FIRST
    family = LAGRANGE
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
  [./strain_xx]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_xx
    index = 0
  [../]
  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
  [../]
 []

[BCs]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./y_top]
    type = FunctionPresetBC
    variable = disp_y
    boundary = top
    function = 't/5'
  [../]
[]

[Materials]
  [./constant]
    type = LinearStrainHardening
    block = 1
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
    yield_stress = 2.4e2
    hardening_constant = 0
    relative_tolerance = 1e-9
    absolute_tolerance = 1e-25
    disp_x = disp_x
    disp_y = disp_y
  [../]
[]

[Executioner]
  end_time = 0.2
  dt = 0.005
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
[]

[Postprocessors]
  [./stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  [../]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  [../]
  [./disp_y]
    type = NodalSum
    variable = disp_y
    boundary = top
  [../]
  [./force_y]
    type = NodalSum
    variable = force_y
    boundary = top
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
  print_perf_log = true
[]
