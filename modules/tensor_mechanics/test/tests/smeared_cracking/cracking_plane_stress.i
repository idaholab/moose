################################################################################
#
# 1x1x1 cube, single element
# simulate plane stress
# pull in +y direction on right surface to produce shear strain
#
#
#
#          ____________
#         /|          /|
#        / |  5      / |                       -X  Left   1
#       /__________ /  |                       +X  Right  4
#      |   |    3  |   |                       +Y  Top    5
#      | 1 |       | 4 |                       -Y  Bottom 2
#      |   |_6_____|___|           y           +Z  Front  6
#      |  /        |  /            ^           -Z  Back   3
#      | /    2    | /             |
#      |/__________|/              |
#                                  ----> x
#                                 /
#                                /
#                               z
#
#
#
#################################################################################

[Mesh]
  file = cube.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0.0026 0 -0.0026 0'
  [../]
  [./pressure]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0   0    0   0'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx'
  [../]
[]

[BCs]
  [./pull_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = displ
  [../]
  [./pin_x]
    type = DirichletBC
    variable = disp_x
    boundary = '1  4'
    value = 0.0
  [../]
  [./pin_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./back]
    type = DirichletBC
    variable = disp_z
    boundary = '3'
    value = 0.0
  [../]
  [./front]
    type = Pressure
    variable = disp_z
    boundary = 6
    function = pressure
    factor   = 1.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200.0e3
    poissons_ratio = .3
  [../]
  [./elastic_stress]
    type = ComputeSmearedCrackingStress
    cracking_stress = 120
    shear_retention_factor = 0.1
    softening_models = exponential_softening
  [../]
  [./exponential_softening]
    type = ExponentialSoftening
    residual_stress = 0.1
    beta = 0.1
  [../]
[]

[Executioner]
  type = Transient
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type'
  petsc_options_value = '101                asm      lu'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
  l_tol = 1e-5
  start_time = 0.0
  end_time = 0.4
  dt = 0.04
[]

[Outputs]
  exodus = true
[]
