#
# Test to exercise the exponential stress release
#
# Stress vs. strain should show a linear relationship until cracking,
#   an exponential stress release, a linear relationship back to zero
#   strain, a linear relationship with the original stiffness in
#   compression and then back to zero strain, a linear relationship
#   back to the exponential curve, and finally further exponential
#   stress release.

[Mesh]
  file = cracking_rz_test.e
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./disply]
    type = PiecewiseLinear
    x = '0 1       2  3      4 5       6'
    y = '0 0.00175 0 -0.0001 0 0.00175 0.0035'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
    use_automatic_differentiation = true
  [../]
[]

[BCs]
  [./pully]
    type = ADFunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = disply
  [../]
  [./bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]

  [./sides]
    type = ADDirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 186.5e9
    poissons_ratio = 0.316
  [../]
  [./elastic_stress]
    type = ADComputeSmearedCrackingStress
    cracking_stress = 119.3e6
    softening_models = exponential_softening
  [../]
  [./exponential_softening]
    type = ADExponentialSoftening
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton

  petsc_options_iname = '-ksp_gmres_restart -pc_type'
  petsc_options_value = '101                lu'

  line_search = 'none'
  l_max_its = 100
  l_tol = 1e-5

  nl_max_its = 10
  nl_rel_tol = 1e-8

  nl_abs_tol = 1e-4

  start_time = 0.0
  end_time = 6.0
  dt = 0.005
  dtmin = 0.005
[]

[Outputs]
  exodus = true
[]
