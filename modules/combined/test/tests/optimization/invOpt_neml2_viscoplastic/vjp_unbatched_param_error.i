[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
  skip_nl_system_check = true
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [adjoint_disp_x]
    solver_sys = adjoint
  []
  [adjoint_disp_y]
    solver_sys = adjoint
  []
  [adjoint_disp_z]
    solver_sys = adjoint
  []
[]

[AuxVariables]
  [elasticity_E]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [all]
        strain = SMALL
        new_system = true
        formulation = TOTAL
        volumetric_locking_correction = false
      []
    []
  []
[]

[NEML2]
  input = 'viscoplasticity.i'
  device = 'cpu'
  [all]
    model = 'model'
    parameter_types = 'FUNCTION FUNCTION FUNCTION FUNCTION'
    parameters = 'elasticity_E yield_sy Xrate_C flow_rate_eta'
    derivatives = 'neml2_stress neml2_strain'
    parameter_vjp_variable = 'neml2_stress'
    parameter_vjp_cotangent = 'adjoint_strain_sr2'
    parameter_vjp_parameters = 'elasticity_E'
    execution_order_group = -1
    execute_on = 'LINEAR NONLINEAR TIMESTEP_END ADJOINT_TIMESTEP_END'
  []
[]

[BCs]
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
  [zfix]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [pull]
    type = NeumannBC
    variable = disp_z
    boundary = 'front'
    value = 40
  []
[]

[Materials]
  [convert_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'mechanical_strain'
    to = 'neml2_strain'
  []
  [stress]
    type = ComputeLagrangianObjectiveCustomSymmetricStress
    custom_small_stress = 'neml2_stress'
    custom_small_jacobian = 'dneml2_stress/dneml2_strain'
  []
  [adjoint_strain]
    type = ComputeLagrangianStrain
    displacements = 'adjoint_disp_x adjoint_disp_y adjoint_disp_z'
    base_name = 'adjoint'
  []
  [convert_adjoint_strain]
    type = RankTwoTensorToSymmetricRankTwoTensor
    from = 'adjoint_mechanical_strain'
    to = 'adjoint_strain_sr2'
  []
[]

[Functions]
  [yield_sy]
    type = ParsedOptimizationFunction
    expression = '85 * sigma_y'
    param_symbol_names = 'sigma_y'
    param_vector_name = 'params/sigma_y'
  []
  [Xrate_C]
    type = ParsedOptimizationFunction
    expression = '8.0e3 * C'
    param_symbol_names = 'C'
    param_vector_name = 'params/C'
  []
  [flow_rate_eta]
    type = ParsedOptimizationFunction
    expression = '400 * eta'
    param_symbol_names = 'eta'
    param_vector_name = 'params/eta'
  []
[]

[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'sigma_y C eta'
    real_vector_values = '1; 1; 1'
  []
[]

[Preconditioning]
  [nl0]
    type = SMP
    nl_sys = 'nl0'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    full = true
  []
  [adjoint]
    type = SMP
    nl_sys = 'adjoint'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    full = true
  []
[]

[Executioner]
  type = TransientAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint

  dt = 1
  dtmin = 1
  num_steps = 1

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  console = false
[]
