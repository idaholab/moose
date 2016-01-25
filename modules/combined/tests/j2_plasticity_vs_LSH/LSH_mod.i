# Test designed to compare results and active time between SH/LinearStrainHardening
# material vs TM j2 plastic user object. As number of elements increases, TM
# active time increases at a much higher rate than SM. Testing at 4x4x4
# (64 elements).
#
# Original test located at:
# solid_mechanics/tests/LinearStrainHardening/LinearStrainHardening_test.i

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  zmin = -0.5
  zmax = 0.5
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

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
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
[]

[AuxKernels]
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
  [./strain_yy]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_yy
    index = 1
  [../]
  [./strain_zz]
    type = MaterialTensorAux
    tensor = total_strain
    variable = strain_zz
    index = 2
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
  [./back]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./z]
    type = FunctionPresetBC
    variable = disp_z
    boundary = front
    function = 't/60'
  [../]
[]

[Materials]
  [./constant]
    type = LinearStrainHardening
    block = 0
    youngs_modulus = 2.1e5
    poissons_ratio = 0.3
    yield_stress = 2.4e2
    hardening_constant = 0
    relative_tolerance = 1e-9
    absolute_tolerance = 1e-25
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-10
  l_tol = 1e-4
  start_time = 0.0
  end_time = 0.5
  dt = 0.01
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
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  [../]
  [./strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  [../]
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  print_perf_log = true
[]
