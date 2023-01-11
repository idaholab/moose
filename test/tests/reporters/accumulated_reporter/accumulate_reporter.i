[Mesh/mesh]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Functions/fun]
  type = ParsedFunction
  expression = 't * x'
[]

[Postprocessors/pp]
  type = FunctionValuePostprocessor
  function = fun
  point = '1 0 0'
  execute_on = 'initial timestep_end'
[]

[VectorPostprocessors/vpp]
  type = LineFunctionSampler
  functions = fun
  start_point = '0 0 0'
  end_point = '1 0 0'
  num_points = 6
  sort_by = x
  execute_on = 'initial timestep_end'
[]

[Reporters]
  [rep]
    type = ConstantReporter
    dof_id_type_names  = 'dofid'
    dof_id_type_values = '1'
    integer_names  = 'int'
    integer_values = '1'
    string_names  = 'str'
    string_values = 'two'
    integer_vector_names  = 'int_vec'
    integer_vector_values = '3 4'
    string_vector_names  = 'str_vec'
    string_vector_values = 'five six seven eight'
    dof_id_type_vector_names  = 'dofid_vec'
    dof_id_type_vector_values = '1 2 3'
    outputs = none
  []
  [accumulate]
    type = AccumulateReporter
    reporters = 'pp/value vpp/fun rep/int rep/str rep/int_vec rep/str_vec rep/dofid rep/dofid_vec'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5

  # This is just testing that AccumulateReporter doesn't accumulate picard iterations
  fixed_point_max_its = 3
  custom_pp = pp
  direct_pp_value = true
  disable_fixed_point_residual_norm_check = true
  accept_on_max_fixed_point_iteration = true
[]

[Outputs]
  [out]
    type = JSON
  []
[]
