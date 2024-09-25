#
# Martensitic transformation
# Chemical driving force described by Landau Polynomial
# Coupled with elasticity (Mechanics)
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100

  elem_type = QUAD4
[]

[Variables]
  [./eta1]
    [./InitialCondition]
      type = RandomIC
      min = 0
      max = 0.1
    [../]
  [../]
  [./eta2]
    [./InitialCondition]
      type = RandomIC
      min = 0
      max = 0.1
    [../]
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    generate_output = 'stress_xx stress_yy'
    eigenstrain_names = 'eigenstrain1 eigenstrain2'
  [../]
[]

[Kernels]
  [./eta_bulk1]
    type = AllenCahn
    variable = eta1
    args = 'eta2'
    f_name = F
  [../]
  [./eta_bulk2]
    type = AllenCahn
    variable = eta2
    args = 'eta1'
    f_name = F
  [../]
  [./eta_interface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa_eta
  [../]
  [./eta_interface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa_eta
  [../]

  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L kappa_eta'
    prop_values = '1 1'
  [../]

  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    property_name = Fc
    coupled_variables = 'eta1 eta2'
    constant_names = 'A2 A3 A4'
    constant_expressions = '0.2 -12.6 12.4'
    expression = 'A2/2*(eta1^2+eta2^2) + A3/3*(eta1^3+eta2^3) + A4/4*(eta1^2+eta2^2)^2'
    enable_jit = true
    derivative_order = 2
  [../]

  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '700 300 300 700 300 700 300 300 300'
    fill_method = symmetric9
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./var_dependence1]
    type = DerivativeParsedMaterial
    property_name = var_dep1
    coupled_variables = 'eta1'
    expression = eta1
    enable_jit = true
    derivative_order = 2
  [../]
  [./var_dependence2]
    type = DerivativeParsedMaterial
    property_name = var_dep2
    coupled_variables = 'eta2'
    expression = eta2
    enable_jit = true
    derivative_order = 2
  [../]

  [./eigenstrain1]
    type = ComputeVariableEigenstrain
    eigen_base = '0.1 -0.1 0 0 0 0'
    prefactor = var_dep1
    args = 'eta1'
    eigenstrain_name = eigenstrain1
  [../]

  [./eigenstrain2]
    type = ComputeVariableEigenstrain
    eigen_base = '-0.1 0.1 0 0 0 0'
    prefactor = var_dep2
    args = 'eta2'
    eigenstrain_name = eigenstrain2
  [../]

  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'eta1 eta2'
    derivative_order = 2
  [../]

  [./totol_free_energy]
    type = DerivativeSumMaterial
    property_name = F
    sum_materials = 'Fc Fe'
    coupled_variables = 'eta1 eta2'
    derivative_order = 2
  [../]
[]

[BCs]
  [./all_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'top bottom left right'
    value = 0
  [../]
  [./all_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'top bottom left right'
    value = 0
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  # this gives best performance on 4 cores
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type  -sub_pc_type '
  petsc_options_value = 'asm       lu'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 10

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 9
    iteration_window = 2
    growth_factor = 1.1
    cutback_factor = 0.75
    dt = 0.3
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
