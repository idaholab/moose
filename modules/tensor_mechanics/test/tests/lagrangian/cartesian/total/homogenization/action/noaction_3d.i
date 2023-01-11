# 3D test with just mixed stress strain control

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  constraint_types = 'stress strain strain strain stress strain strain strain strain'
  macro_gradient = hvar
  homogenization_constraint = homogenization
[]

[Mesh]
  [base]
    type = FileMeshGenerator
    file = '3d.exo'
  []

  [sidesets]
    type = SideSetsFromNormalsGenerator
    input = base
    normals = '-1 0 0
                1 0 0
                0 -1 0
                0 1 0
            '
              '    0 0 -1
                0 0 1'
    fixed_normal = true
    new_boundary = 'left right bottom top back front'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [hvar]
    family = SCALAR
    order = NINTH
  []
[]

[AuxVariables]
  [pk1_stress_xx]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_yx]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_zx]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_xy]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_yy]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_zy]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_xz]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_yz]
    family = MONOMIAL
    order = CONSTANT
  []
  [pk1_stress_zz]
    family = MONOMIAL
    order = CONSTANT
  []

  [deformation_gradient_xx]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yx]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zx]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_xy]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yy]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zy]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_xz]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yz]
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zz]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pk1_stress_xx]
    type = RankTwoAux
    variable = pk1_stress_xx
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 0
  []
  [pk1_stress_yx]
    type = RankTwoAux
    variable = pk1_stress_yx
    rank_two_tensor = pk1_stress
    index_i = 1
    index_j = 0
  []
  [pk1_stress_zx]
    type = RankTwoAux
    variable = pk1_stress_zx
    rank_two_tensor = pk1_stress
    index_i = 2
    index_j = 0
  []
  [pk1_stress_xy]
    type = RankTwoAux
    variable = pk1_stress_xy
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 1
  []
  [pk1_stress_yy]
    type = RankTwoAux
    variable = pk1_stress_yy
    rank_two_tensor = pk1_stress
    index_i = 1
    index_j = 1
  []
  [pk1_stress_zy]
    type = RankTwoAux
    variable = pk1_stress_zy
    rank_two_tensor = pk1_stress
    index_i = 2
    index_j = 1
  []
  [pk1_stress_xz]
    type = RankTwoAux
    variable = pk1_stress_xz
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 2
  []
  [pk1_stress_yz]
    type = RankTwoAux
    variable = pk1_stress_yz
    rank_two_tensor = pk1_stress
    index_i = 1
    index_j = 2
  []
  [pk1_stress_zz]
    type = RankTwoAux
    variable = pk1_stress_zz
    rank_two_tensor = pk1_stress
    index_i = 2
    index_j = 2
  []

  [deformation_gradient_xx]
    type = RankTwoAux
    variable = deformation_gradient_xx
    rank_two_tensor = deformation_gradient
    index_i = 0
    index_j = 0
  []
  [deformation_gradient_yx]
    type = RankTwoAux
    variable = deformation_gradient_yx
    rank_two_tensor = deformation_gradient
    index_i = 1
    index_j = 0
  []
  [deformation_gradient_zx]
    type = RankTwoAux
    variable = deformation_gradient_zx
    rank_two_tensor = deformation_gradient
    index_i = 2
    index_j = 0
  []
  [deformation_gradient_xy]
    type = RankTwoAux
    variable = deformation_gradient_xy
    rank_two_tensor = deformation_gradient
    index_i = 0
    index_j = 1
  []
  [deformation_gradient_yy]
    type = RankTwoAux
    variable = deformation_gradient_yy
    rank_two_tensor = deformation_gradient
    index_i = 1
    index_j = 1
  []
  [deformation_gradient_zy]
    type = RankTwoAux
    variable = deformation_gradient_zy
    rank_two_tensor = deformation_gradient
    index_i = 2
    index_j = 1
  []
  [deformation_gradient_xz]
    type = RankTwoAux
    variable = deformation_gradient_xz
    rank_two_tensor = deformation_gradient
    index_i = 0
    index_j = 2
  []
  [deformation_gradient_yz]
    type = RankTwoAux
    variable = deformation_gradient_yz
    rank_two_tensor = deformation_gradient
    index_i = 1
    index_j = 2
  []
  [deformation_gradient_zz]
    type = RankTwoAux
    variable = deformation_gradient_zz
    rank_two_tensor = deformation_gradient
    index_i = 2
    index_j = 2
  []
[]

[UserObjects]
  [homogenization]
    type = HomogenizationConstraint
    targets = 'stress11 strain21 strain31 strain12 stress22 strain32 strain13 strain23 strain33'
    execute_on = 'INITIAL LINEAR NONLINEAR'
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
  [sdz]
    type = HomogenizedTotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[ScalarKernels]
  [enforce]
    type = HomogenizationConstraintScalarKernel
    variable = hvar
  []
[]

[Functions]
  [stress11]
    type = ParsedFunction
    expression = '120.0*t'
  []
  [stress22]
    type = ParsedFunction
    expression = '65*t'
  []
  [strain33]
    type = ParsedFunction
    expression = '8.0e-2*t'
  []
  [strain23]
    type = ParsedFunction
    expression = '2.0e-2*t'
  []
  [strain13]
    type = ParsedFunction
    expression = '-7.0e-2*t'
  []
  [strain12]
    type = ParsedFunction
    expression = '1.0e-2*t'
  []
  [strain32]
    type = ParsedFunction
    expression = '1.0e-2*t'
  []
  [strain31]
    type = ParsedFunction
    expression = '2.0e-2*t'
  []
  [strain21]
    type = ParsedFunction
    expression = '-1.5e-2*t'
  []
  [zero]
    type = ConstantFunction
    expression = 0
  []
[]

[BCs]
  [Periodic]
    [x]
      variable = disp_x
      auto_direction = 'x y z'
    []
    [y]
      variable = disp_y
      auto_direction = 'x y z'
    []
    [z]
      variable = disp_z
      auto_direction = 'x y z'
    []
  []

  [fix1_x]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_x
    value = 0
  []
  [fix1_y]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_y
    value = 0
  []
  [fix1_z]
    type = DirichletBC
    boundary = "fix_all"
    variable = disp_z
    value = 0
  []

  [fix2_x]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_x
    value = 0
  []
  [fix2_y]
    type = DirichletBC
    boundary = "fix_xy"
    variable = disp_y
    value = 0
  []

  [fix3_z]
    type = DirichletBC
    boundary = "fix_z"
    variable = disp_z
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

[Preconditioning]
  [smp]
    type = SMP
    full = true
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
  nl_max_its = 20
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 1.0
[]

[Outputs]
  file_base = 3d
  exodus = true
[]
