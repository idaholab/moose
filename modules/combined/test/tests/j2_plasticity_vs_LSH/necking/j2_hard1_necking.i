#

[Mesh]
  file = necking_quad4.e
  displacements = 'disp_x disp_y'
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
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
#    save_in_disp_x = force_x
    save_in_disp_y = force_y
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
  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
#  [./force_x]
#    order = FIRST
#    family = LAGRANGE
#  [../]
  [./force_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
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
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./y_top]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = 't/5'
  [../]
[]

[UserObjects]
  [./str]
    type = TensorMechanicsHardeningConstant
    value = 2.4e2
  [../]
  [./j2]
    type = TensorMechanicsPlasticJ2
    yield_strength = str
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-9
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 1
    fill_method = symmetric_isotropic
    #with E = 2.1e5 and nu = 0.3
    #changed to SM values using E-nu to Lambda-G
    C_ijkl = '121154 80769.2'
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 1
    displacements = 'disp_x disp_y'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 1
    ep_plastic_tolerance = 1E-9
    plastic_models = j2
  [../]
[]

[Executioner]
  end_time = 0.2
  dt = 0.005
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10
  l_tol = 1e-9
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
  [./strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  [../]
  [./strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  [../]
  [./disp_y]
    type = NodalSum
    variable = disp_y
    boundary = top
  [../]
  [./force_y]
    type = NodalSum
    variable = force_y
    boundary = top
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
  perf_graph = true
[]
