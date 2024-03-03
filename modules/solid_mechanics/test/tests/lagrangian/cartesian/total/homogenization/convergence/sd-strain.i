# 2D test with just strain control

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
  constraint_types = 'strain none none strain strain none strain strain strain'
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
    order = SIXTH
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    variable = disp_x
    min = -0.1
    max = 0.1
  []
  [disp_y]
    type = RandomIC
    variable = disp_y
    min = -0.1
    max = 0.1
  []
  [disp_z]
    type = RandomIC
    variable = disp_z
    min = -0.1
    max = 0.1
  []
  [hvar]
    type = ScalarConstantIC
    variable = hvar
    value = 0.1
  []
[]

[AuxVariables]
  [sxx]
    family = MONOMIAL
    order = CONSTANT
  []
  [syy]
    family = MONOMIAL
    order = CONSTANT
  []
  [sxy]
    family = MONOMIAL
    order = CONSTANT
  []
  [szz]
    family = MONOMIAL
    order = CONSTANT
  []
  [syz]
    family = MONOMIAL
    order = CONSTANT
  []
  [sxz]
    family = MONOMIAL
    order = CONSTANT
  []
  [exx]
    family = MONOMIAL
    order = CONSTANT
  []
  [eyy]
    family = MONOMIAL
    order = CONSTANT
  []
  [exy]
    family = MONOMIAL
    order = CONSTANT
  []
  [ezz]
    family = MONOMIAL
    order = CONSTANT
  []
  [eyz]
    family = MONOMIAL
    order = CONSTANT
  []
  [exz]
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
  [syy]
    type = RankTwoAux
    variable = syy
    rank_two_tensor = pk1_stress
    index_i = 1
    index_j = 1
  []
  [sxy]
    type = RankTwoAux
    variable = sxy
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 1
  []

  [zz]
    type = RankTwoAux
    variable = szz
    rank_two_tensor = pk1_stress
    index_i = 2
    index_j = 2
  []
  [syz]
    type = RankTwoAux
    variable = syz
    rank_two_tensor = pk1_stress
    index_i = 1
    index_j = 2
  []
  [sxz]
    type = RankTwoAux
    variable = sxz
    rank_two_tensor = pk1_stress
    index_i = 0
    index_j = 2
  []

  [exx]
    type = RankTwoAux
    variable = exx
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 0
  []
  [eyy]
    type = RankTwoAux
    variable = eyy
    rank_two_tensor = mechanical_strain
    index_i = 1
    index_j = 1
  []
  [exy]
    type = RankTwoAux
    variable = exy
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 1
  []

  [ezz]
    type = RankTwoAux
    variable = ezz
    rank_two_tensor = mechanical_strain
    index_i = 2
    index_j = 2
  []
  [eyz]
    type = RankTwoAux
    variable = eyz
    rank_two_tensor = mechanical_strain
    index_i = 1
    index_j = 2
  []
  [exz]
    type = RankTwoAux
    variable = exz
    rank_two_tensor = mechanical_strain
    index_i = 0
    index_j = 2
  []
[]

[UserObjects]
  [homogenization]
    type = HomogenizationConstraint
    targets = 'strain11 strain12 strain22 strain13 strain23 strain33'
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
  [strain11]
    type = ParsedFunction
    expression = '4.0e-2*t'
  []
  [strain22]
    type = ParsedFunction
    expression = '-2.0e-2*t'
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

[Postprocessors]
  [sxx]
    type = ElementAverageValue
    variable = sxx
    execute_on = 'initial timestep_end'
  []
  [syy]
    type = ElementAverageValue
    variable = syy
    execute_on = 'initial timestep_end'
  []
  [sxy]
    type = ElementAverageValue
    variable = sxy
    execute_on = 'initial timestep_end'
  []

  [szz]
    type = ElementAverageValue
    variable = szz
    execute_on = 'initial timestep_end'
  []
  [syz]
    type = ElementAverageValue
    variable = syz
    execute_on = 'initial timestep_end'
  []
  [sxz]
    type = ElementAverageValue
    variable = sxz
    execute_on = 'initial timestep_end'
  []

  [exx]
    type = ElementAverageValue
    variable = exx
    execute_on = 'initial timestep_end'
  []
  [eyy]
    type = ElementAverageValue
    variable = eyy
    execute_on = 'initial timestep_end'
  []
  [exy]
    type = ElementAverageValue
    variable = exy
    execute_on = 'initial timestep_end'
  []

  [ezz]
    type = ElementAverageValue
    variable = ezz
    execute_on = 'initial timestep_end'
  []
  [eyz]
    type = ElementAverageValue
    variable = eyz
    execute_on = 'initial timestep_end'
  []
  [exz]
    type = ElementAverageValue
    variable = exz
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  #automatic_scaling = true

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.2
  dtmin = 0.2
  end_time = 0.2
[]

[Outputs]
  exodus = false
  csv = false
[]
