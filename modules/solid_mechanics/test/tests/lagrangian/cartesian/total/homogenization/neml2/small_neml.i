constraint_types = 'strain none none none none none none none none'
targets = 'strain11 zero zero zero zero zero zero zero zero'

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

[BCs]
  [fix1_x]
    type = DirichletBC
    boundary = fix_all
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_x
    vector_tags = residual
  []
[]

[BCs]
  [fix1_y]
    type = DirichletBC
    boundary = fix_all
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_y
    vector_tags = residual
  []
[]

[BCs]
  [fix1_z]
    type = DirichletBC
    boundary = fix_all
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_z
    vector_tags = residual
  []
[]

[BCs]
  [fix2_x]
    type = DirichletBC
    boundary = fix_xy
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_x
    vector_tags = residual
  []
[]

[BCs]
  [fix2_y]
    type = DirichletBC
    boundary = fix_xy
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_y
    vector_tags = residual
  []
[]

[BCs]
  [fix3_z]
    type = DirichletBC
    boundary = fix_z
    displacements = 'disp_x disp_y disp_z'
    matrix_tags = 'system time'
    value = 0
    variable = disp_z
    vector_tags = residual
  []
[]

[Functions]
  [strain11]
    type = ParsedFunction
    expression = t
  []
[]

[Functions]
  [zero]
    type = ConstantFunction
    value = 0
  []
[]

[Materials]
  [compute_homogenization_gradient]
    type = ComputeHomogenizedLagrangianStrain
    constraint_types = ${constraint_types}
    targets = ${targets}
    macro_gradient = hvar
  []
  [compute_strain]
    type = ComputeLagrangianStrain
    homogenization_gradient_names = 'homogenization_gradient'
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Materials]
  [neml2_stress_to_moose]
    type = NEML2ToMOOSESymmetricRankTwoTensorMaterialProperty
    block = ''
    from_neml2 = state/S
    neml2_executor = neml2_model_all
    outputs = none
    to_moose = neml2_stress
  []
  [neml2_jacobian_to_moose]
    type = NEML2ToMOOSESymmetricRankFourTensorMaterialProperty
    block = ''
    from_neml2 = state/S
    neml2_executor = neml2_model_all
    neml2_input_derivative = forces/E
    outputs = none
    to_moose = neml2_jacobian
  []
[]

[Materials]
  [convert_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = mechanical_strain
    outputs = none
    to = neml2_strain
  []
[]

[Materials]
  [stress]
    type = ComputeLagrangianObjectiveCustomSymmetricStress
    custom_small_jacobian = neml2_jacobian
    custom_small_stress = neml2_stress
    large_kinematics = false
    outputs = none
  []
[]

[UserObjects]
  [moose_strain_to_jacobian]
    type = MOOSESymmetricRankTwoTensorMaterialPropertyToNEML2
    block = ''
    execute_on = 'INITIAL LINEAR NONLINEAR'
    from_moose = neml2_strain
    to_neml2 = forces/E
  []
  [neml2_index_model_all]
    type = NEML2BatchIndexGenerator
    block = ''
    execute_on = 'INITIAL LINEAR NONLINEAR'
  []
  [neml2_model_all]
    type = NEML2ModelExecutor
    batch_index_generator = neml2_index_model_all
    device = cpu
    execute_on = 'INITIAL LINEAR NONLINEAR'
    gatherers = moose_strain_to_jacobian
    input = neml2_elastic.i
    model = model
    param_gatherers = ''
  []
[]

[Variables]
  [disp_x]
    type = MooseVariable
    family = LAGRANGE
    order = FIRST
  []
  [disp_y]
    type = MooseVariable
    family = LAGRANGE
    order = FIRST
  []
  [disp_z]
    type = MooseVariable
    family = LAGRANGE
    order = FIRST
  []
  [hvar]
    type = MooseVariableScalar
    family = SCALAR
    order = FIRST
  []
[]

[AuxKernels]
  [cauchy_stress_xx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_xx
    variable = cauchy_stress_xx
  []
  [cauchy_stress_xy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_xy
    variable = cauchy_stress_xy
  []
  [cauchy_stress_xz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_xz
    variable = cauchy_stress_xz
  []
  [cauchy_stress_yx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_yx
    variable = cauchy_stress_yx
  []
  [cauchy_stress_yy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_yy
    variable = cauchy_stress_yy
  []
  [cauchy_stress_yz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_yz
    variable = cauchy_stress_yz
  []
  [cauchy_stress_zx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_zx
    variable = cauchy_stress_zx
  []
  [cauchy_stress_zy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_zy
    variable = cauchy_stress_zy
  []
  [cauchy_stress_zz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = cauchy_stress_zz
    variable = cauchy_stress_zz
  []
  [deformation_gradient_xx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_xx
    variable = deformation_gradient_xx
  []
  [deformation_gradient_xy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_xy
    variable = deformation_gradient_xy
  []
  [deformation_gradient_xz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_xz
    variable = deformation_gradient_xz
  []
  [deformation_gradient_yx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_yx
    variable = deformation_gradient_yx
  []
  [deformation_gradient_yy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_yy
    variable = deformation_gradient_yy
  []
  [deformation_gradient_yz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_yz
    variable = deformation_gradient_yz
  []
  [deformation_gradient_zx_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_zx
    variable = deformation_gradient_zx
  []
  [deformation_gradient_zy_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_zy
    variable = deformation_gradient_zy
  []
  [deformation_gradient_zz_all]
    type = MaterialRealAux
    block = ''
    execute_on = TIMESTEP_END
    property = deformation_gradient_zz
    variable = deformation_gradient_zz
  []
[]

[AuxVariables]
  [cauchy_stress_xx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_xy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_xz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_yx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_yy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_yz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_zx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_zy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [cauchy_stress_zz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_xx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_xy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_xz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_yz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zx]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zy]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
  [deformation_gradient_zz]
    type = MooseVariableConstMonomial
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [TM_all0]
    type = HomogenizedTotalLagrangianStressDivergence
    component = 0
    displacements = 'disp_x disp_y disp_z'
    large_kinematics = false
    stabilize_strain = false
    variable = disp_x
    macro_var = hvar
    constraint_types = ${constraint_types}
    targets = ${targets}
  []
  [TM_all1]
    type = HomogenizedTotalLagrangianStressDivergence
    component = 1
    displacements = 'disp_x disp_y disp_z'
    large_kinematics = false
    stabilize_strain = false
    variable = disp_y
    macro_var = hvar
    constraint_types = ${constraint_types}
    targets = ${targets}
  []
  [TM_all2]
    type = HomogenizedTotalLagrangianStressDivergence
    component = 2
    displacements = 'disp_x disp_y disp_z'
    large_kinematics = false
    stabilize_strain = false
    variable = disp_z
    macro_var = hvar
    constraint_types = ${constraint_types}
    targets = ${targets}
  []
[]

[Materials]
  [cauchy_stress_xx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 0
    outputs = none
    property_name = cauchy_stress_xx
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_xy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 1
    outputs = none
    property_name = cauchy_stress_xy
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_xz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 2
    outputs = none
    property_name = cauchy_stress_xz
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_yx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 0
    outputs = none
    property_name = cauchy_stress_yx
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_yy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 1
    outputs = none
    property_name = cauchy_stress_yy
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_yz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 2
    outputs = none
    property_name = cauchy_stress_yz
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_zx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 0
    outputs = none
    property_name = cauchy_stress_zx
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_zy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 1
    outputs = none
    property_name = cauchy_stress_zy
    rank_two_tensor = cauchy_stress
  []
  [cauchy_stress_zz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 2
    outputs = none
    property_name = cauchy_stress_zz
    rank_two_tensor = cauchy_stress
  []
  [deformation_gradient_xx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 0
    outputs = none
    property_name = deformation_gradient_xx
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_xy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 1
    outputs = none
    property_name = deformation_gradient_xy
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_xz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 0
    index_j = 2
    outputs = none
    property_name = deformation_gradient_xz
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_yx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 0
    outputs = none
    property_name = deformation_gradient_yx
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_yy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 1
    outputs = none
    property_name = deformation_gradient_yy
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_yz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 1
    index_j = 2
    outputs = none
    property_name = deformation_gradient_yz
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_zx_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 0
    outputs = none
    property_name = deformation_gradient_zx
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_zy_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 1
    outputs = none
    property_name = deformation_gradient_zy
    rank_two_tensor = deformation_gradient
  []
  [deformation_gradient_zz_all]
    type = RankTwoCartesianComponent
    block = ''
    index_i = 2
    index_j = 2
    outputs = none
    property_name = deformation_gradient_zz
    rank_two_tensor = deformation_gradient
  []
[]

[Executioner]
  type = Transient

  residual_and_jacobian_together = true

  solve_type = 'newton'
  line_search = 'none'

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
  [out]
    type = Exodus
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [mCS_xx]
    type     = ElementAverageValue
    variable = cauchy_stress_xx
  []
  [mCS_yy]
    type     = ElementAverageValue
    variable = cauchy_stress_yy
  []
  [mCS_zz]
    type     = ElementAverageValue
    variable = cauchy_stress_zz
  []
  [mCS_xy]
    type     = ElementAverageValue
    variable = cauchy_stress_xy
  []
  [mCS_xz]
    type     = ElementAverageValue
    variable = cauchy_stress_xz
  []
  [mCS_yx]
    type     = ElementAverageValue
    variable = cauchy_stress_yx
  []
  [mCS_yz]
    type     = ElementAverageValue
    variable = cauchy_stress_yz
  []
  [mCS_zy]
    type     = ElementAverageValue
    variable = cauchy_stress_zy
  []
  [mCS_zx]
    type     = ElementAverageValue
    variable = cauchy_stress_zx
  []
[]
