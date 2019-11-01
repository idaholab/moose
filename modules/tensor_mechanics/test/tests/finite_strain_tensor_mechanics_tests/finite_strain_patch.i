# Patch Test
# This test is designed to compute constant xx, yy, zz, xy, yz, and zx
# stress on a set of irregular hexes.  The mesh is composed of one
# block with seven elements.  The elements form a unit cube with one
# internal element.  There is a nodeset for each exterior node.
# The cube is displaced by 1e-6 units in x, 2e-6 in y, and 3e-6 in z.
# The faces are sheared as well (1e-6, 2e-6, and 3e-6 for xy, yz, and
# zx).  This gives a uniform strain/stress state for all six unique
# tensor components.
# With Young's modulus at 1e6 and Poisson's ratio at 0, the shear
# modulus is 5e5 (G=E/2/(1+nu)).  Therefore,
#
# stress xx = 1e6 * 1e-6 = 1
# stress yy = 1e6 * 2e-6 = 2
# stress zz = 1e6 * 3e-6 = 3
# stress xy = 2 * 5e5 * 1e-6 / 2 = 0.5
# (2 * G   * gamma_xy / 2 = 2 * G * epsilon_xy)
# stress yz = 2 * 5e5 * 2e-6 / 2 = 1
# stress zx = 2 * 5e5 * 3e-6 / 2 = 1.5

[Mesh]
  # Comment
  # Mesh
  file = patch_mesh.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  # Functions
  [./rampConstant1]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 1e-6
  [../]
  [./rampConstant2]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 2e-6
  [../]
  [./rampConstant3]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 3e-6
  [../]
  [./rampConstant4]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 4e-6
  [../]
  [./rampConstant6]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 1. 1.'
    scale_factor = 6e-6
  [../]
[]

[Variables]
  # Variables
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
  # AuxVariables
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

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  # AuxKernels
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
  [./stress_yz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yz
    index_i = 1
    index_j = 2
  [../]
  [./stress_zx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zx
    index_i = 2
    index_j = 0
  [../]
[]

[BCs]
  # BCs
  [./node1_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./node1_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 1
    function = rampConstant2
  [../]
  [./node1_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 1
    function = rampConstant3
  [../]
  [./node2_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 2
    function = rampConstant1
  [../]
  [./node2_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = rampConstant2
  [../]
  [./node2_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 2
    function = rampConstant6
  [../]
  [./node3_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 3
    function = rampConstant1
  [../]
  [./node3_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./node3_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 3
    function = rampConstant3
  [../]
  [./node4_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = 0.0
  [../]
  [./node4_y]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = 0.0
  [../]
  [./node4_z]
    type = DirichletBC
    variable = disp_z
    boundary = 4
    value = 0.0
  [../]
  [./node5_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 5
    function = rampConstant1
  [../]
  [./node5_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 5
    function = rampConstant4
  [../]
  [./node5_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 5
    function = rampConstant3
  [../]
  [./node6_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 6
    function = rampConstant2
  [../]
  [./node6_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 6
    function = rampConstant4
  [../]
  [./node6_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 6
    function = rampConstant6
  [../]
  [./node7_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 7
    function = rampConstant2
  [../]
  [./node7_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 7
    function = rampConstant2
  [../]
  [./node7_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 7
    function = rampConstant3
  [../]
  [./node8_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 8
    function = rampConstant1
  [../]
  [./node8_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 8
    function = rampConstant2
  [../]
  [./node8_z]
    type = DirichletBC
    variable = disp_z
    boundary = 8
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1 2 3 4 5 6 7'
    C_ijkl = '1.0e6  0.0   0.0 1.0e6  0.0  1.0e6 0.5e6 0.5e6 0.5e6'
    fill_method = symmetric9
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = '1 2 3 4 5 6 7'
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2 3 4 5 6 7'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  # Executioner
  type = Transient

  solve_type = 'NEWTON'


  nl_abs_tol = 1e-10
  l_max_its = 20
  start_time = 0.0
  dt = 1.0
  num_steps = 2
  petsc_options_iname = -pc_type
  petsc_options_value = lu
  end_time = 2.0
[]

[Outputs]
  exodus = true
[] # Output
