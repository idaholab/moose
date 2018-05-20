[Mesh]
  # Comment
  type = CohesiveZoneMeshSplit
  file = coh3D_3Blocks.e
  split_interface = true
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

  [./sxx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./syy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./szz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./syz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sxz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sxy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]

  [./sxx]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = sxx
    block = '1 2 3'
  []
  [./syy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = syy
    block = '1 2 3'
  []
  [./szz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = szz
    block = '1 2 3'
  []
  [./syz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = syz
    block = '1 2 3'
  []
  [./sxz]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 2
    variable = sxz
    block = '1 2 3'
  []
  [./sxy]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = sxy
    block = '1 2 3'
  []

[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]


  [./top3_x]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0.0
  [../]
  [./top3_y]
    type = DirichletBC
    variable = disp_y
    boundary = 3
    value = 0.0
  [../]
  [./top3_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 3
    function = 0.01*t
  [../]

[]

[InterfaceKernels]
  [./interface_x]
    type = DisplacementJumpBasedCohesiveInterfaceKernel
    variable = disp_x
    neighbor_var = disp_x
    disp_1 = disp_y
    disp_1_neighbor = disp_y
    disp_2 = disp_z
    disp_2_neighbor = disp_z
    disp_index = 0
    boundary = 'czm_bM1_bS2 czm_bM1_bS3 czm_bM2_bS3'

  [../]
  [./interface_y]
    type = DisplacementJumpBasedCohesiveInterfaceKernel
    variable = disp_y
    neighbor_var = disp_y
    disp_1 = disp_x
    disp_1_neighbor = disp_x
    disp_2 = disp_z
    disp_2_neighbor = disp_z
    disp_index = 1
    boundary = 'czm_bM1_bS2 czm_bM1_bS3 czm_bM2_bS3'
  [../]
  [./interface_z]
    type = DisplacementJumpBasedCohesiveInterfaceKernel
    variable = disp_z
    neighbor_var = disp_z
    disp_1 = disp_x
    disp_1_neighbor = disp_x
    disp_2 = disp_y
    disp_2_neighbor = disp_y
    disp_index = 2
    boundary = 'czm_bM1_bS2 czm_bM1_bS3 czm_bM2_bS3'
  [../]
[]

[UserObjects]
  [./TractionSeparationTest_12]
    type = CohesiveLaw_3DC
    boundary = 'czm_bM1_bS2'
    DeltaU0 = '1 0.5'
    MaxAllowableTraction = '1e2 5e1'
  [../]
  [./TractionSeparationTest_13]
    type = CohesiveLaw_3DC
    boundary = 'czm_bM1_bS3'
    DeltaU0 = '1 0.5'
    MaxAllowableTraction = '2e2 1e2'
  [../]
  [./TractionSeparationTest_23]
    type = CohesiveLaw_3DC
    boundary = 'czm_bM2_bS3'
    DeltaU0 = '1 0.5'
    MaxAllowableTraction = '5e1 2.5e1'
  [../]
[]

[Materials]
  [./Elasticity_tensor]
    type = ComputeElasticityTensor
    block = '1 2 3'
    fill_method = symmetric_isotropic
    C_ijkl = '0.3 0.5e8'
  [../]
  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    block = '1 2 3'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2 3'
  [../]
  [./interface_12]
    type = DisplacementJumpBasedCohesiveInterfaceMaterial
    boundary = 'czm_bM1_bS2'
    uo_TractionSeparationLaw = 'TractionSeparationTest_12'
    disp_x = disp_x
    disp_x_neighbor = disp_x
    disp_y = disp_y
    disp_y_neighbor = disp_y
    disp_z = disp_z
    disp_z_neighbor = disp_z
  [../]
  [./interface_13]
    type = DisplacementJumpBasedCohesiveInterfaceMaterial
    boundary = 'czm_bM1_bS3'
    uo_TractionSeparationLaw = 'TractionSeparationTest_13'
    disp_x = disp_x
    disp_x_neighbor = disp_x
    disp_y = disp_y
    disp_y_neighbor = disp_y
    disp_z = disp_z
    disp_z_neighbor = disp_z
  [../]
  [./interface_23]
    type = DisplacementJumpBasedCohesiveInterfaceMaterial
    boundary = 'czm_bM2_bS3'
    uo_TractionSeparationLaw = 'TractionSeparationTest_23'
    disp_x = disp_x
    disp_x_neighbor = disp_x
    disp_y = disp_y
    disp_y_neighbor = disp_y
    disp_z = disp_z
    disp_z_neighbor = disp_z
  [../]
[]

 [Preconditioning]
   [./SMP]
     type = SMP
     full = true
   [../]
 []

[Executioner]
  # Preconditisoned JFNK (default)
  type = Transient

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # petsc_options_value = 'hypre     boomerang'

  solve_type = newton
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
  nl_max_its = 5
  l_tol = 1e-10
  l_max_its = 50
  start_time = 0.0
  dt = 10.0
  end_time = 300
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]

[Postprocessors]
  [./sxx_3G]
    type = ElementAverageValue
    variable = sxx
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./syy_3G]
    type = ElementAverageValue
    variable = syy
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./szz_3G]
    type = ElementAverageValue
    variable = szz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./syz_3G]
    type = ElementAverageValue
    variable = syz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./sxz_3G]
    type = ElementAverageValue
    variable = sxz
    execute_on = 'initial timestep_end'
    block = 3
  [../]
  [./sxy_3G]
    type = ElementAverageValue
    variable = sxy
    execute_on = 'initial timestep_end'
    block = 3
  [../]


[]
