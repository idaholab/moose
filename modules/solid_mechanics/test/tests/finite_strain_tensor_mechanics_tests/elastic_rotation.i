#
# Rotation Test
#
# This test is designed to compute a uniaxial stress and then follow that
# stress as the mesh is rotated 90 degrees.
#
# The mesh is composed of one block with a single element.  The nodal
# displacements in the x and y directions are prescribed.  Poisson's
# ratio is zero.
#

[Mesh]
  file = rotation_test.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Functions]
  # Functions
  [./x_200]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2*(t-t0)) - 1.0)'
  [../]
  [./y_200]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, 0.0, (1.0+delta)*sin(pi/2*(t-t0)))'
  [../]
  [./x_300]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, delta*t, (1.0+delta)*cos(pi/2.0*(t-t0)) - sin(pi/2.0*(t-t0)) - 1.0)'
  [../]
  [./y_300]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) + (1+delta)*sin(pi/2.0*(t-t0)) - 1.0)'
  [../]
  [./x_400]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, 0.0, -sin(pi/2.0*(t-t0)))'
  [../]
  [./y_400]
    type = ParsedFunction
    symbol_names = 'delta t0'
    symbol_values = '-1e-6 1.0'
    expression = 'if(t<=1.0, 0.0, cos(pi/2.0*(t-t0)) - 1.0)'
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
  [./stress_xy]
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
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  [../]
[]

[BCs]
  # BCs
  [./no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 100
    value = 0.0
  [../]
  [./no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 100
    value = 0.0
  [../]
  [./x_200]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 200
    function = x_200
  [../]
  [./y_200]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 200
    function = y_200
  [../]
  [./x_300]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 300
    function = x_300
  [../]
  [./y_300]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 300
    function = y_300
  [../]
  [./x_400]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 400
    function = x_400
  [../]
  [./y_400]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 400
    function = y_400
  [../]
  [./no_z]
    type = DirichletBC
    variable = disp_z
    boundary = '100 200 300 400'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    C_ijkl = '1.0e6  0.0   0.0 1.0e6  0.0  1.0e6 0.5e6 0.5e6 0.5e6'
    fill_method = symmetric9
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 1
    displacements = 'disp_x disp_y disp_z'
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]
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
  [./stress_xy]
    type = ElementAverageValue
    variable = stress_xy
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

  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-30
  nl_abs_tol = 1e-20
  l_max_its = 20
  start_time = 0.0
  dt = 0.01
  end_time = 2.0
[]

[Outputs]
  exodus = true
[] # Outputs
