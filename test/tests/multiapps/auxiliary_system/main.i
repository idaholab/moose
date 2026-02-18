[MultiApps]
  [fracture]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = 'TIMESTEP_END'
    clone_parent_mesh = false
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[Variables]
  [a]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = 'a'
  []
[]

[AuxVariables]
  [time_aux_output]
    family = MONOMIAL
    order = CONSTANT
  []
  [time_var]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = FunctionAux
      function = 't'
    []
  []
[]

[AuxKernels]
  [target_d_avg]
    type = ADMaterialRealAux
    variable = time_aux_output
    property = time
    execute_on = 'TIMESTEP_END'
  []
[]

[Materials]
  [sample_mat]
    type = ADParsedMaterial
    property_name = time
    expression = 'time_var'
    coupled_variables = 'time_var'
    outputs = exodus
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist                 '
  automatic_scaling = true

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  dt = 1e-6
  end_time = 4e-6

  fixed_point_max_its = 20
  accept_on_max_fixed_point_iteration = true
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-10
  fixed_point_min_its = 5
[]

# [Problem]
#   solve = false
# []

[Outputs]
  print_linear_residuals = false
  [exodus]
    type = Exodus
  []
  csv = true
[]

# These two should match
[Postprocessors]
  [variable_value]
    type = ElementExtremeValue
    variable = 'time_var'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [matprop_auto_output]
    type = ElementExtremeValue
    variable = 'time'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [matprop_manual_output]
    type = ElementExtremeValue
    variable = 'time_aux_output'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
