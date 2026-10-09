[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [b0]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    subdomain_ids = '0'
    boundary_id_offset = 0
    boundary_name_prefix = 'b0'
  []
  [b1]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    subdomain_ids = '1'
    boundary_id_offset = 10
    boundary_name_prefix = 'b1'
  []
  [b2]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    subdomain_ids = '2'
    boundary_id_offset = 20
    boundary_name_prefix = 'b2'
  []
  [b3]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    subdomain_ids = '3'
    boundary_id_offset = 30
    boundary_name_prefix = 'b3'
  []
  [b4]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    subdomain_ids = '4'
    boundary_id_offset = 40
    boundary_name_prefix = 'b4'
  []
  [stack]
    type = CombinerGenerator
    inputs = 'b0 b1 b2 b3 b4'
    positions = '0 0 0
                 0 0 3
                 0 0 6
                 0 0 9
                 0 0 12'
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
  [dummy]
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
    parameter_derivatives = 'neml2_stress elasticity_E;
                             neml2_stress yield_sy;
                             neml2_stress Xrate_C;
                             neml2_stress flow_rate_eta'
  []
[]

[BCs]
  [xfix]
    type = DirichletBC
    variable = disp_x
    boundary = 'b0_left b1_left b2_left b3_left b4_left'
    value = 0
  []
  [yfix]
    type = DirichletBC
    variable = disp_y
    boundary = 'b0_bottom b1_bottom b2_bottom b3_bottom b4_bottom'
    value = 0
  []
  [zfix]
    type = DirichletBC
    variable = disp_z
    boundary = 'b0_back b1_back b2_back b3_back b4_back'
    value = 0
  []
  [pull0]
    type = NeumannBC
    variable = disp_z
    boundary = 'b0_front'
    value = 40
  []
  [pull1]
    type = NeumannBC
    variable = disp_z
    boundary = 'b1_front'
    value = 80
  []
  [pull2]
    type = NeumannBC
    variable = disp_z
    boundary = 'b2_front'
    value = 120
  []
  [pull3]
    type = NeumannBC
    variable = disp_z
    boundary = 'b3_front'
    value = 160
  []
  [pull4]
    type = NeumannBC
    variable = disp_z
    boundary = 'b4_front'
    value = 200
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
[]

[Functions]
  [elasticity_E]
    type = ParsedOptimizationFunction
    expression = '8.0e4 * E'
    param_symbol_names = 'E'
    param_vector_name = 'params/E'
  []
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
  [measure_data]
    type = OptimizationData
    variable = disp_z
    objective_name = objective_value
    measurement_file = 'measurements.csv'
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'E sigma_y C eta'
    real_vector_values = '1; 1; 1; 1'
  []
[]

[DiracKernels]
  [misfit_is_adjoint_force]
    type = ReporterPointSource
    variable = adjoint_disp_z
    x_coord_name = measure_data/measurement_xcoord
    y_coord_name = measure_data/measurement_ycoord
    z_coord_name = measure_data/measurement_zcoord
    value_name = measure_data/misfit_values
  []
[]

[VectorPostprocessors]
  [grad_E]
    type = AdjointStrainSymmetricStressGradInnerProduct
    stress_derivative_name = 'dneml2_stress/delasticity_E'
    adjoint_strain_name = 'adjoint_mechanical_strain'
    variable = dummy
    function = elasticity_E
    execute_on = ADJOINT_TIMESTEP_END
  []
  [grad_sigma_y]
    type = AdjointStrainSymmetricStressGradInnerProduct
    stress_derivative_name = 'dneml2_stress/dyield_sy'
    adjoint_strain_name = 'adjoint_mechanical_strain'
    variable = dummy
    function = yield_sy
    execute_on = ADJOINT_TIMESTEP_END
  []
  [grad_C]
    type = AdjointStrainSymmetricStressGradInnerProduct
    stress_derivative_name = 'dneml2_stress/dXrate_C'
    adjoint_strain_name = 'adjoint_mechanical_strain'
    variable = dummy
    function = Xrate_C
    execute_on = ADJOINT_TIMESTEP_END
  []
  [grad_eta]
    type = AdjointStrainSymmetricStressGradInnerProduct
    stress_derivative_name = 'dneml2_stress/dflow_rate_eta'
    adjoint_strain_name = 'adjoint_mechanical_strain'
    variable = dummy
    function = flow_rate_eta
    execute_on = ADJOINT_TIMESTEP_END
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

  residual_and_jacobian_together = false
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  console = false
[]
