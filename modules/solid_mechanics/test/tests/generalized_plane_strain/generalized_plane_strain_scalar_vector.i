[Mesh]
  file = 2squares.e
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./scalar_strain_zz1]
    order = FIRST
    family = SCALAR
  [../]
  [./scalar_strain_zz2]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]
  [./saved_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./saved_y]
    order = FIRST
    family = LAGRANGE
  [../]

  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
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

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./aux_strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Postprocessors]
  [./react_z1]
    type = MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    block = 1
  [../]
  [./react_z2]
    type = MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    block = 2
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./GeneralizedPlaneStrain]
      [./gps1]
        use_displaced_mesh = true
        displacements = 'disp_x disp_y'
        scalar_out_of_plane_strain = scalar_strain_zz1
        block = '1'
      [../]
      [./gps2]
        use_displaced_mesh = true
        displacements = 'disp_x disp_y'
        scalar_out_of_plane_strain = scalar_strain_zz2
        block = '2'
      [../]
    [../]
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = false
    displacements = 'disp_x disp_y'
    temperature = temp
    save_in = 'saved_x saved_y'
    block = '1 2'
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./stress_xy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
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

  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_xy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
  [./aux_strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = aux_strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = '(1-x)*t'
  [../]
[]

[BCs]
  [./bottom1x]
    type = DirichletBC
    boundary = 1
    variable = disp_x
    value = 0.0
  [../]
  [./bottom1y]
    type = DirichletBC
    boundary = 1
    variable = disp_y
    value = 0.0
  [../]
  [./bottom2x]
    type = DirichletBC
    boundary = 2
    variable = disp_x
    value = 0.0
  [../]
  [./bottom2y]
    type = DirichletBC
    boundary = 2
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    block = '1 2'
  [../]
  [./strain1]
    type = ComputePlaneSmallStrain
    displacements = 'disp_x disp_y'
    subblock_index_provider = test_subblock_index_provider
    scalar_out_of_plane_strain = 'scalar_strain_zz1 scalar_strain_zz2'
    block = '1 2'
    eigenstrain_names = eigenstrain
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.5
    block = '1 2'
    eigenstrain_name = eigenstrain
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]
[]

[UserObjects]
  [./test_subblock_index_provider]
    type = TestSubblockIndexProvider
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-10

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
  num_steps = 5000
[]

[Outputs]
  exodus = true
[]
