# Simple 3D test

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [angles]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0 1.5707963'
  []

  [stretch]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0.1 0.1'
  []

  [move_y]
    type = ParsedFunction
    expression = 'y*cos(theta) - z * (1 + a)*sin(theta) - y'
    symbol_names = 'a theta'
    symbol_values = 'stretch angles'
  []

  [move_z]
    type = ParsedFunction
    expression = 'y*sin(theta) + z*(1+a)*cos(theta) - z'
    symbol_names = 'a theta'
    symbol_values = 'stretch angles'
  []

  [dts]
    type = PiecewiseConstant
    x = '0 1 2'
    y = '0.1 0.001 0.001'
    direction = 'LEFT_INCLUSIVE'
  []
[]

[BCs]
  [fix]
    type = DirichletBC
    preset = true
    value = 0.0
    boundary = left
    variable = disp_x
  []

  [front_y]
    type = FunctionDirichletBC
    boundary = front
    variable = disp_y
    function = move_y
    preset = true
  []

  [back_y]
    type = FunctionDirichletBC
    boundary = back
    variable = disp_y
    function = move_y
    preset = true
  []

  [front_z]
    type = FunctionDirichletBC
    boundary = front
    variable = disp_z
    function = move_z
    preset = true
  []

  [back_z]
    type = FunctionDirichletBC
    boundary = back
    variable = disp_z
    function = move_z
    preset = true
  []

[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = timestep_end
  []
  [stress_zz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_zz
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [stress_xy]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xy
    index_i = 0
    index_j = 1
    execute_on = timestep_end
  []
  [stress_xz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_xz
    index_i = 0
    index_j = 2
    execute_on = timestep_end
  []
  [stress_yz]
    type = RankTwoAux
    rank_two_tensor = cauchy_stress
    variable = stress_yz
    index_i = 1
    index_j = 2
    execute_on = timestep_end
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [sxx]
    type = ElementAverageValue
    variable = stress_xx
  []
  [syy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [szz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [syz]
    type = ElementAverageValue
    variable = stress_yz
  []
  [sxz]
    type = ElementAverageValue
    variable = stress_xz
  []
  [sxy]
    type = ElementAverageValue
    variable = stress_xy
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-6

  start_time = 0.0
  end_time = 2.0
  [TimeStepper]
    type = FunctionDT
    function = dts
    interpolate = False
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
