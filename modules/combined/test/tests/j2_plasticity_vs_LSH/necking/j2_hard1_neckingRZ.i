#

[Mesh]
  file = necking_quad4.e
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [./disp_r]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./AxisymmetricRZ]
    use_displaced_mesh = true
#    save_in_disp_r = force_r
    save_in_disp_z = force_z
  [../]
[]

[AuxVariables]
  [./stress_rr]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_rr]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
#  [./force_r]
#    order = FIRST
#    family = LAGRANGE
#  [../]
  [./force_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./stress_rr]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rr
    index_i = 0
    index_j = 0
  [../]
  [./stress_zz]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 1
    index_j = 1
  [../]
  [./strain_rr]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_rr
    index_i = 0
    index_j = 0
  [../]
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 1
    index_j = 1
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = disp_r
    boundary = left
    value = 0.0
  [../]
  [./bottom]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./z]
    type = FunctionDirichletBC
    variable = disp_z
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
    type = ComputeAxisymmetricRZFiniteStrain
    block = 1
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = 1
    ep_plastic_tolerance = 1E-9
    plastic_models = j2
  [../]
[]

[Executioner]
  end_time = 0.1
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
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-15
  l_tol = 1e-9
[]

[Postprocessors]
  [./stress_rr]
    type = ElementAverageValue
    variable = stress_rr
  [../]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./strain_rr]
    type = ElementAverageValue
    variable = strain_rr
  [../]
  [./strain_zz]
    type = ElementAverageValue
    variable = strain_zz
  [../]
  [./disp_z]
    type = NodalSum
    variable = disp_z
    boundary = top
  [../]
  [./force_z]
    type = NodalSum
    variable = force_z
    boundary = top
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
  perf_graph = true
[]
