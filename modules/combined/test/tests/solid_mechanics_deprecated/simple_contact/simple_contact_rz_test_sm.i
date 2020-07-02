#
# The analytic solution is:
#   disp_x = -7e-5 * x
#   disp_y =  6e-5 * y
#   stress_xx = stress_zz = -100
#   stress_yy = stress_xy = 0
#
# Note: Run merged_rz.i to generate a solution to compare to that doesn't use contact.

[Mesh]
  file = contact_rz.e
  # PETSc < 3.5.0 requires the iteration patch_update_strategy to
  # avoid PenetrationLocator warnings, which are currently treated as
  # errors by the TestHarness.
  patch_update_strategy = 'iteration'
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[Problem]
  coord_type = RZ
[]

[Functions]
  [./pressure]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = 100
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
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
[]

[SolidMechanics]
  [./solid]
    disp_r = disp_x
    disp_z = disp_y
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 3
    secondary = 2
    penalty = 1e5
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

  [./stress_zz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zz
    index = 2
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]

  [./stress_yz]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yz
    index = 4
  [../]

  [./stress_zx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_zx
    index = 5
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 10
    value = 0.0
  [../]

  [./Pressure]
    [./right_pressure]
      boundary = 4
      function = pressure
      disp_x = disp_x
      disp_y = disp_y
    [../]
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    formulation = NonlinearRZ

    block = 1

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]

  [./stiffStuff2]
    type = Elastic
    formulation = NonlinearRZ

    block = 2

    disp_r = disp_x
    disp_z = disp_y

    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu       '

  line_search = 'none'

  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-9

  l_max_its = 20
  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
