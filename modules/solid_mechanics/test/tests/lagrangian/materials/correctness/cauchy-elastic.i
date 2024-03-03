# Simple 3D test

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[Functions]
  [strain]
    type = ParsedFunction
    expression = 't'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    preset = true
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [boty]
    type = DirichletBC
    preset = true
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [backz]
    type = DirichletBC
    preset = true
    boundary = back
    variable = disp_z
    value = 0.0
  []

  [pull_x]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = strain
  []
[]

[Materials]
  [elastic_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 100000.0
    poissons_ratio = 0.3
  []
  [compute_stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [compute_strain]
    type = ComputeLagrangianStrain
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
    rank_two_tensor = cauchy_stress
    index_i = 0
    index_j = 0
  []
  [syy]
    type = RankTwoAux
    variable = syy
    rank_two_tensor = cauchy_stress
    index_i = 1
    index_j = 1
  []
  [sxy]
    type = RankTwoAux
    variable = sxy
    rank_two_tensor = cauchy_stress
    index_i = 0
    index_j = 1
  []

  [zz]
    type = RankTwoAux
    variable = szz
    rank_two_tensor = cauchy_stress
    index_i = 2
    index_j = 2
  []
  [syz]
    type = RankTwoAux
    variable = syz
    rank_two_tensor = cauchy_stress
    index_i = 1
    index_j = 2
  []
  [sxz]
    type = RankTwoAux
    variable = sxz
    rank_two_tensor = cauchy_stress
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

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 2
  l_tol = 1e-14
  nl_max_its = 5
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 0.1
  dtmin = 0.1
  end_time = 0.1
[]

[Outputs]
  exodus = false
  csv = true
[]
