[GlobalParams]
  displacements = 'disp_x disp_y'
  temperature = temp

  poissons_ratio = 0.3
  youngs_modulus = 1e6
  thermal_expansion_coeff = 0.0002
  stress_free_temperature = 0.0
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./temp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./tstrain_xx]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tstrain_yy]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tstrain_zz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tstrain_xy]
    order = FIRST
    family = LAGRANGE
  [../]

  [./mstrain_xx]
    order = FIRST
    family = LAGRANGE
  [../]
  [./mstrain_yy]
    order = FIRST
    family = LAGRANGE
  [../]
  [./mstrain_zz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./mstrain_xy]
    order = FIRST
    family = LAGRANGE
  [../]


  [./stress_xx]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_yy]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
  [./stress_xy]
    order = FIRST
    family = LAGRANGE
  [../]

  [./von_mises]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Modules/Peridynamics/Mechanics/Master]
  [./all]
    formulation = ORDINARY_STATE
  [../]
[]

[AuxKernels]
  [./tempfuncaux]
    type = FunctionAux
    variable = temp
    function = tempfunc
    use_displaced_mesh = false
  [../]

  [./tstrain_xx]
    type = NodalRankTwoPD
    variable = tstrain_xx
    rank_two_tensor = total_strain
    output_type = component
    index_i = 0
    index_j = 0
  [../]
  [./tstrain_yy]
    type = NodalRankTwoPD
    variable = tstrain_yy
    rank_two_tensor = total_strain
    output_type = component
    index_i = 1
    index_j = 1
  [../]
  [./tstrain_zz]
    type = NodalRankTwoPD
    variable = tstrain_zz
    rank_two_tensor = total_strain
    output_type = component
    index_i = 2
    index_j = 2
  [../]
  [./tstrain_xy]
    type = NodalRankTwoPD
    variable = tstrain_xy
    rank_two_tensor = total_strain
    output_type = component
    index_i = 0
    index_j = 1
  [../]

  [./mstrain_xx]
    type = NodalRankTwoPD
    variable = mstrain_xx
    rank_two_tensor = mechanical_strain
    output_type = component
    index_i = 0
    index_j = 0
  [../]
  [./mstrain_yy]
    type = NodalRankTwoPD
    variable = mstrain_yy
    rank_two_tensor = mechanical_strain
    output_type = component
    index_i = 1
    index_j = 1
  [../]
  [./mstrain_zz]
    type = NodalRankTwoPD
    variable = mstrain_zz
    rank_two_tensor = mechanical_strain
    output_type = component
    index_i = 2
    index_j = 2
  [../]
  [./mstrain_xy]
    type = NodalRankTwoPD
    variable = mstrain_xy
    rank_two_tensor = mechanical_strain
    output_type = component
    index_i = 0
    index_j = 1
  [../]

  [./stress_xx]
    type = NodalRankTwoPD
    variable = stress_xx
    rank_two_tensor = stress
    output_type = component
    index_i = 0
    index_j = 0
  [../]
  [./stress_yy]
    type = NodalRankTwoPD
    variable = stress_yy
    rank_two_tensor = stress
    output_type = component
    index_i = 1
    index_j = 1
  [../]
  [./stress_zz]
    type = NodalRankTwoPD
    variable = stress_zz
    rank_two_tensor = stress
    output_type = component
    index_i = 2
    index_j = 2
  [../]
  [./stress_xy]
    type = NodalRankTwoPD
    variable = stress_xy
    rank_two_tensor = stress
    output_type = component
    index_i = 0
    index_j = 1
  [../]

  [./vonmises]
    type = NodalRankTwoPD
    variable = von_mises
    rank_two_tensor = stress
    output_type = scalar
    scalar_type = VonMisesStress
  [../]
[]

[Functions]
  [./tempfunc]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    boundary = 1003
    variable = disp_x
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    boundary = 1000
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
  [../]

  [./force_density]
    type = ComputeSmallStrainConstantHorizonMaterialOSPD
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = planestrain_thermomechanics_ranktwotensor_OSPD
[]
