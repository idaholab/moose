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
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
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

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    incremental = true
    strain = FINITE
    generate_output = 'stress_xx stress_xy stress_zx stress_yy stress_zz stress_yz'
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 3
    secondary = 2
    penalty = 1e5
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
    [../]
  [../]
[]

[Materials]
  [./stiffStuff]
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  [../]
  [./stiffStuff_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
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
