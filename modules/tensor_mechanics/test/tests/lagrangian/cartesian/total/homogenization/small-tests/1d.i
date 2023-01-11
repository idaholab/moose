# 1D strain controlled test

[GlobalParams]
  displacements = 'disp_x'
  large_kinematics = false
  macro_gradient = hvar
  homogenization_constraint = homogenization
[]

[Mesh]
  [base]
    type = FileMeshGenerator
    file = '1d.exo'
  []
  [ss]
    type = SideSetsFromPointsGenerator
    input = base
    points = '-1 0 0
               7 0 0'
    new_boundary = 'left right'
  []
[]

[Variables]
  [disp_x]
  []
  [hvar]
    family = SCALAR
    order = FIRST
  []
[]

[AuxVariables]
  [sxx]
    family = MONOMIAL
    order = CONSTANT
  []
  [exx]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sxx]
    type = RankTwoAux
    variable = sxx
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 0
  []
  [exx]
    type = RankTwoAux
    variable = exx
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 0
  []
[]

[UserObjects]
  [homogenization]
    type = HomogenizationConstraint
    constraint_types = ${constraint_types}
    targets = ${targets}
    execute_on = 'INITIAL LINEAR NONLINEAR'
  []
[]

[Kernels]
  [sdx]
    type = HomogenizedTotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
[]

[ScalarKernels]
  [enforce]
    type = HomogenizationConstraintScalarKernel
    variable = hvar
  []
[]

[Functions]
  [func_stress]
    type = ParsedFunction
    expression = '1800*t'
  []
  [func_strain]
    type = ParsedFunction
    expression = '1.0e-2*t'
  []
[]

[BCs]
  [Periodic]
    [all]
      variable = disp_x
      auto_direction = 'x'
    []
  []

  [centerfix_x]
    type = DirichletBC
    boundary = "fixme"
    variable = disp_x
    value = 0
  []
[]

[Materials]
  [elastic_tensor_1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
    block = '1'
  []
  [elastic_tensor_2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 120000.0
    poissons_ratio = 0.21
    block = '2'
  []
  [elastic_tensor_3]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 80000.0
    poissons_ratio = 0.4
    block = '3'
  []
  [elastic_tensor_4]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 76000.0
    poissons_ratio = 0.11
    block = '4'
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
    homogenization_gradient_names = 'homogenization_gradient'
  []
  [compute_homogenization_gradient]
    type = ComputeHomogenizedLagrangianStrain
  []
[]

[Postprocessors]
  [sxx]
    type = ElementAverageValue
    variable = sxx
    execute_on = 'initial timestep_end'
  []
  [exx]
    type = ElementAverageValue
    variable = exx
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = default

  automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 15
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8

  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 1.0
[]

[Outputs]
  exodus = false
  csv = true
[]
