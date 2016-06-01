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
  [./displx]
    type = PiecewiseLinear
#   x = '0 1'
#   y = '0 .0035'
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

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    use_displaced_mesh = false
  [../]
[]


[AuxKernels]

  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_j = 0
    index_i = 0
    execute_on = timestep_end
    block = 1
  [../]

  [./stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = stress
    index_j = 1
    index_i = 1
    execute_on = timestep_end
    block = 1
  [../]

  [./stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = stress
    index_j = 2
    index_i = 2
    execute_on = timestep_end
    block = 1
  [../]

  [./stress_xy]
    type = RankTwoAux
    variable = stress_xy
    rank_two_tensor = stress
    index_j = 1
    index_i = 0
    execute_on = timestep_end
    block = 1
  [../]

  [./stress_yz]
    type = RankTwoAux
    variable = stress_yz
    rank_two_tensor = stress
    index_j = 2
    index_i = 1
    execute_on = timestep_end
    block = 1
  [../]

  [./stress_zx]
    type = RankTwoAux
    variable = stress_zx
    rank_two_tensor = stress
    index_j = 2
    index_i = 0
    execute_on = timestep_end
    block = 1
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
    block = 1
    youngs_modulus = 186.5e9
    poissons_ratio = .316
  [../]

  [./strain]
    type = ComputeSmallStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./elastic_stress]
    type = ComputeElasticSmearedCrackingStress
    block = 1
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
