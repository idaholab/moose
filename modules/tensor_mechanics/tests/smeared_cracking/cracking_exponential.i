#
# Test to exercise the exponential stress release
#
# Stress vs. strain should show a linear relationship until cracking,
#   an exponential stress release, a linear relationship back to zero
#   strain, a linear relationship with the original stiffness in
#   compression and then back to zero strain, a linear relationship
#   back to the exponential curve, and finally further exponential
#   stress release.
#

[Mesh]
  file = cracking_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./displx]
    type = PiecewiseLinear
    x = '0 1       2  3      4 5       6'
    y = '0 0.00175 0 -0.0001 0 0.00175 0.0035'
  [../]
  [./disply]
    type = PiecewiseLinear
    x = '0 5 6'
    y = '0 0 .00175'
  [../]
  [./displz]
    type = PiecewiseLinear
    x = '0 2 3'
    y = '0 0 .0035'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  [./pullx]
    type = FunctionPresetBC
    #type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = displx
  [../]
  [./left]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]

  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = '11 12'
    value = 0.0
  [../]

  [./move_y]
    type = FunctionPresetBC
    variable = disp_y
    boundary = '15 16'
    function = disply
  [../]

  [./back]
    type = PresetBC
    variable = disp_z
    boundary = '3'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 186.5e9
    poissons_ratio = .316
  [../]
  [./elastic_stress]
    type = ComputeElasticSmearedCrackingStress
    cracking_release = exponential
    cracking_stress = 119.3e6
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'

  line_search = 'none'
  l_max_its = 100
  l_tol = 1e-6

  nl_max_its = 10
  nl_rel_tol = 1e-12

  # Some timesteps in this example start from very small initial
  # residuals, so it is legitimate to use a (very small) nl_abs_tol.
  nl_abs_tol = 1.e-20

  start_time = 0.0
  dt = 0.005
  dtmin = 0.005
  num_steps = 1200
[]

[Outputs]
  exodus = true
[]
