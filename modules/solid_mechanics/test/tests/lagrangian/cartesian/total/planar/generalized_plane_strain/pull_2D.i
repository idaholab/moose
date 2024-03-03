constraint_types = 'none none none none none none none none strain'
targets = '0'

[GlobalParams]
  displacements = 'disp_x disp_y'
  large_kinematics = true
  stabilize_strain = true
  macro_gradient = hvar
  homogenization_constraint = homogenization
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
  use_displaced_mesh = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [hvar]
    family = SCALAR
    order = FIRST
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

[ScalarKernels]
  [enforce]
    type = HomogenizationConstraintScalarKernel
    variable = hvar
  []
[]

[Kernels]
  [sdx]
    type = HomogenizedTotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = HomogenizedTotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'top bottom'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = 't'
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1000.0
    poissons_ratio = 0.25
  []
  [compute_homogenization_gradient]
    type = ComputeHomogenizedLagrangianStrain
  []
  [compute_strain]
    type = ComputeLagrangianStrain
    homogenization_gradient_names = 'homogenization_gradient'
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [stress_zz]
    type = RankTwoCartesianComponent
    rank_two_tensor = cauchy_stress
    index_i = 2
    index_j = 2
    property_name = stress_zz
  []
  [strain_zz]
    type = RankTwoCartesianComponent
    rank_two_tensor = mechanical_strain
    index_i = 2
    index_j = 2
    property_name = strain_zz
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 0.1

  solve_type = 'newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [strain_zz]
    type = ElementAverageMaterialProperty
    mat_prop = strain_zz
  []
  [stress_zz]
    type = ElementAverageMaterialProperty
    mat_prop = stress_zz
  []
[]

[Outputs]
  csv = true
[]
