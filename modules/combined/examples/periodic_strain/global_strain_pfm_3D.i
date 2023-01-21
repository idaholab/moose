[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    ny = 20
    nz = 20
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
  [./cnode]
    input = gen
    type = ExtraNodesetGenerator
    coord = '0.0 0.0 0.0'
    new_boundary = 100
  [../]
[]

[Variables]
  [./u_x]
  [../]
  [./u_y]
  [../]
  [./u_z]
  [../]
  [./global_strain]
    order = SIXTH
    family = SCALAR
  [../]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = 'sin(2*x*pi)*sin(2*y*pi)*sin(2*z*pi)*0.05+0.6'
    [../]
  [../]
  [./w]
  [../]
[]

[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
  [./s00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./s11]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e00]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e01]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e10]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e11]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./disp_x]
    type = GlobalDisplacementAux
    variable = disp_x
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 0
  [../]
  [./disp_y]
    type = GlobalDisplacementAux
    variable = disp_y
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 1
  [../]
  [./disp_z]
    type = GlobalDisplacementAux
    variable = disp_z
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
    component = 2
  [../]
  [./local_free_energy]
    type = TotalFreeEnergy
    execute_on = 'initial LINEAR'
    variable = local_energy
    interfacial_vars = 'c'
    kappa_names = 'kappa_c'
  [../]
  [./s00]
    type = RankTwoAux
    variable = s00
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
  [../]
  [./s01]
    type = RankTwoAux
    variable = s01
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
  [../]
  [./s10]
    type = RankTwoAux
    variable = s10
    rank_two_tensor = stress
    index_i = 1
    index_j = 0
  [../]
  [./s11]
    type = RankTwoAux
    variable = s11
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
  [../]
  [./e00]
    type = RankTwoAux
    variable = e00
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
  [../]
  [./e01]
    type = RankTwoAux
    variable = e01
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 1
  [../]
  [./e10]
    type = RankTwoAux
    variable = e10
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 0
  [../]
  [./e11]
    type = RankTwoAux
    variable = e11
    rank_two_tensor = total_strain
    index_i = 1
    index_j = 1
  [../]
[]

[GlobalParams]
  derivative_order = 2
  enable_jit = true
  displacements = 'u_x u_y u_z'
  block = 0
[]

[Kernels]
  [./TensorMechanics]
  [../]

  # Cahn-Hilliard kernels
  [./c_dot]
    type = CoupledTimeDerivative
    variable = w
    v = c
    block = 0
  [../]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
    block = 0
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
    block = 0
  [../]
[]

[ScalarKernels]
  [./global_strain]
    type = GlobalStrain
    variable = global_strain
    global_strain_uo = global_strain_uo
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y z'
      variable = 'c w u_x u_y u_z'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = DirichletBC
    boundary = 100
    variable = u_x
    value = 0
  [../]
  [./centerfix_y]
    type = DirichletBC
    boundary = 100
    variable = u_y
    value = 0
  [../]
  [./centerfix_z]
    type = DirichletBC
    boundary = 100
    variable = u_z
    value = 0
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M   kappa_c'
    prop_values = '0.2 0.01   '
  [../]

  [./shear1]
    type = GenericConstantRankTwoTensor
    tensor_values = '0 0 0 0.5 0.5 0.5'
    tensor_name = shear1
  [../]
  [./shear2]
    type = GenericConstantRankTwoTensor
    tensor_values = '0 0 0 -0.5 -0.5 -0.5'
    tensor_name = shear2
  [../]
  [./expand3]
    type = GenericConstantRankTwoTensor
    tensor_values = '1 1 1 0 0 0'
    tensor_name = expand3
  [../]

  [./weight1]
    type = DerivativeParsedMaterial
    expression = '0.3*c^2'
    property_name = weight1
    coupled_variables = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    expression = '0.3*(1-c)^2'
    property_name = weight2
    coupled_variables = c
  [../]
  [./weight3]
    type = DerivativeParsedMaterial
    expression = '4*(0.5-c)^2'
    property_name = weight3
    coupled_variables = c
  [../]

  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    global_strain = global_strain
    eigenstrain_names = eigenstrain
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    tensors = 'shear1  shear2  expand3'
    weights = 'weight1 weight2 weight3'
    args = c
    eigenstrain_name = eigenstrain
  [../]

  [./global_strain]
    type = ComputeGlobalStrain
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]

  # chemical free energies
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    property_name = Fc
    expression = '4*c^2*(1-c)^2'
    coupled_variables = 'c'
    outputs = exodus
    output_properties = Fc
  [../]

  # elastic free energies
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'c'
    outputs = exodus
    output_properties = Fe
  [../]

  # free energy (chemical + elastic)
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    property_name = F
    sum_materials = 'Fc Fe'
    coupled_variables = 'c'
  [../]
[]

[UserObjects]
  [./global_strain_uo]
    type = GlobalStrainUserObject
    execute_on = 'Initial Linear Nonlinear'
  [../]
[]

[Postprocessors]
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'initial TIMESTEP_END'
    variable = local_energy
  [../]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    execute_on = 'initial TIMESTEP_END'
    variable = c
  [../]
  [./min]
    type = ElementExtremeValue
    execute_on = 'initial TIMESTEP_END'
    value_type = min
    variable = c
  [../]
  [./max]
    type = ElementExtremeValue
    execute_on = 'initial TIMESTEP_END'
    value_type = max
    variable = c
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
  scheme = bdf2
  solve_type = 'PJFNK'

  line_search = basic

  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm         31   preonly   lu      1'

  l_max_its = 30
  nl_max_its = 12

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  end_time = 2.0

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    growth_factor = 1.5
    cutback_factor = 0.8
    optimal_iterations = 9
    iteration_window = 2
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = false
  exodus = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
