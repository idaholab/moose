coupling = 1.0
D_val = 1.0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
  []
[]

[Problem]
  nl_sys_names = 'nl0 adjoint'
  kernel_coverage_check = false
[]

[Variables]
  [u]
    initial_condition = 0
  []
  [v]
    initial_condition = 0
  []
  [u_adj]
    solver_sys = adjoint
    initial_condition = 0
  []
  [v_adj]
    solver_sys = adjoint
    initial_condition = 0
  []
[]

[Kernels]
  [diff_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = D
  []
  [couple_u_v]
    type = ADMatCoupledForce
    variable = u
    v = v
    mat_prop_coef = coupling_mat
  []
  [diff_v]
    type = ADMatDiffusion
    variable = v
    diffusivity = D
  []
  [couple_v_u]
    type = ADMatCoupledForce
    variable = v
    v = u
    mat_prop_coef = coupling_mat
    coef = -1
  []
[]

[Functions]
  [D_func]
    type = ParsedOptimizationFunction
    expression = 'D'
    param_symbol_names = 'D'
    param_vector_name = 'params/D'
  []
[]

[Materials]
  [D_mat]
    type = ADGenericFunctionMaterial
    prop_names = 'D'
    prop_values = 'D_func'
  []
  [coupling_mat]
    type = ADGenericConstantMaterial
    prop_names = 'coupling_mat'
    prop_values = '${coupling}'
  []
[]

[BCs]
  [flux_u]
    type = NeumannBC
    variable = u
    boundary = bottom
    value = 1.0
  []
  [zero_u]
    type = DirichletBC
    variable = u
    boundary = 'top right'
    value = 0
  []
  [zero_v]
    type = DirichletBC
    variable = v
    boundary = 'top right'
    value = 0
  []
[]

[DiracKernels]
  [adj_src]
    type = ReporterPointSource
    variable = u_adj
    x_coord_name = adj_loc/x
    y_coord_name = adj_loc/y
    z_coord_name = adj_loc/z
    value_name = adj_val/value
  []
[]

# ===== TIMESTEP_END execution order chain =====
# Invariant: group=0 PointValues execute before group=1 ParsedPostprocessor.
# If the invariant breaks, combined_angle = atan2(0,0) = 0 instead of the
# correct non-zero angle.
[Postprocessors]
  [u_at_origin]
    type = PointValue
    point = '0 0 0'
    variable = u
    execute_on = TIMESTEP_END
    execution_order_group = 0
    force_preaux = true
  []
  [v_at_origin]
    type = PointValue
    point = '0 0 0'
    variable = v
    execute_on = TIMESTEP_END
    execution_order_group = 0
    force_preaux = true
  []
  [combined_angle]
    type = ParsedPostprocessor
    expression = 'atan2(v_at_origin, u_at_origin)'
    pp_names = 'v_at_origin u_at_origin'
    execute_on = TIMESTEP_END
    execution_order_group = 1
    force_preaux = true
  []
  # ===== ADJOINT_TIMESTEP_END execution order chain =====
  # Invariant: grad_u VPP (group=-1) executes before this PP (group=0) reads it.
  # If the invariant breaks, grad_component reads the stale INITIAL value (0)
  # instead of the correct ADJOINT_TIMESTEP_END inner-product.
  [grad_component]
    type = VectorPostprocessorComponent
    vectorpostprocessor = grad_u
    vector_name = inner_product
    index = 0
    execute_on = ADJOINT_TIMESTEP_END
    execution_order_group = 0
  []
[]

[Reporters]
  [params]
    type = ConstantReporter
    real_vector_names = 'D'
    real_vector_values = '${D_val}'
  []
  [adj_loc]
    type = ConstantReporter
    real_vector_names = 'x y z'
    real_vector_values = '0.5; 0.5; 0'
  []
  [adj_val]
    type = ConstantReporter
    real_vector_names = 'value'
    real_vector_values = '1.0'
  []
  # Reads from group=-1 VPPs; must run at group=1 (after the VPPs).
  [gradient]
    type = ParsedVectorReporter
    name = inner
    vector_reporter_names = 'grad_u/inner_product grad_v/inner_product'
    vector_reporter_symbols = 'gu gv'
    expression = 'gu + gv'
    execute_on = ADJOINT_TIMESTEP_END
    execution_order_group = 1
  []
[]

[VectorPostprocessors]
  [grad_u]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = u_adj
    forward_variable = u
    function = D_func
    execution_order_group = -1
    execute_on = 'INITIAL ADJOINT_TIMESTEP_END'
    outputs = none
  []
  [grad_v]
    type = ElementOptimizationDiffusionCoefFunctionInnerProduct
    variable = v_adj
    forward_variable = v
    function = D_func
    execution_order_group = -1
    execute_on = 'INITIAL ADJOINT_TIMESTEP_END'
    outputs = none
  []
[]

[Preconditioning]
  [nl0]
    type = SMP
    nl_sys = nl0
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
  [adjoint]
    type = SMP
    nl_sys = adjoint
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = SteadyAndAdjoint
  forward_system = nl0
  adjoint_system = adjoint
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
