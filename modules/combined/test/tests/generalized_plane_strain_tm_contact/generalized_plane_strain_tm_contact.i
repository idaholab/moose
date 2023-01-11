[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  scalar_out_of_plane_strain = scalar_strain_zz
  temperature = temp
[]

[Mesh]
  file = 2squares.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./temp]
  [../]
  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
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
  [./strain_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Postprocessors]
  [./react_z]
    type = MaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  [../]
[]

[Kernels]
  [./TensorMechanics]
    use_displaced_mesh = true
  [../]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./GeneralizedPlaneStrain]
      [./gps]
        use_displaced_mesh = true
      [../]
    [../]
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
  [./strain_zz]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
  [../]
[]

[Functions]
  [./tempramp]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[BCs]
  [./x]
    type = DirichletBC
    boundary = '4 6'
    variable = disp_x
    value = 0.0
  [../]
  [./y]
    type = DirichletBC
    boundary = '4 6'
    variable = disp_y
    value = 0.0
  [../]
  [./t]
    type = DirichletBC
    boundary = '4'
    variable = temp
    value = 0.0
  [../]
  [./tramp]
    type = FunctionDirichletBC
    variable = temp
    boundary = '6'
    function = tempramp
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    off_diag_row =    'disp_x disp_y'
    off_diag_column = 'disp_y disp_x'
  [../]
[]

[Contact]
  [./mech]
    primary = 8
    secondary = 2
    penalty = 1e+10
    normalize_penalty = true
    tangential_tolerance = .1
    normal_smoothing_distance = .1
    model = frictionless
    formulation = kinematic
  [../]
[]

[ThermalContact]
  [./thermal]
    type = GapHeatTransfer
    primary = 8
    secondary = 2
    emissivity_primary = 0
    emissivity_secondary = 0
    variable = temp
    tangential_tolerance = .1
    normal_smoothing_distance = .1
    gap_conductivity = 0.01
    min_gap = 0.001
    quadrature = true
  [../]
[]

[Materials]
  [./elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
    block = '1 2'
  [../]
  [./strain]
    type = ComputePlaneSmallStrain
    eigenstrain_names = eigenstrain
    block = '1 2'
  [../]
  [./thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 0.02
    stress_free_temperature = 0.0
    eigenstrain_name = eigenstrain
    block = '1 2'
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = '1 2'
  [../]

  [./heatcond]
    type = HeatConductionMaterial
    thermal_conductivity = 3.0
    specific_heat = 300.0
    block = '1 2'
  [../]

  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  petsc_options_iname = '-pc_type -ps_sub_type -pc_factor_mat_solver_package'
  petsc_options_value = 'asm      lu           superlu_dist'

# controls for linear iterations
  l_max_its = 100
  l_tol = 1e-4

# controls for nonlinear iterations
  nl_max_its = 20
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-10

# time control
  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 2.0
[]

[Outputs]
  exodus = true
[]
