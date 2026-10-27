!include ../../multisystem/picard/nonlinearfe_nonlinearfe/multi_system.i

[AuxVariables]
  [ucopy]
  []
[]

[AuxKernels]
  [ucopy_kernel]
    type = CopyValueAux
    variable = ucopy
    source = u
    execute_on = 'MULTISYSTEM_FIXED_POINT_ITERATION_END'
  []
[]

[FunctorMaterials]
  [u_change_mat]
    type = ADFunctorChangeFunctorMaterial
    functor = u
    change_over = multisystem_fixed_point
    take_absolute_value = false
    prop_name = u_change
  []
  [ucopy_change_mat]
    type = ADFunctorChangeFunctorMaterial
    functor = ucopy
    change_over = multisystem_fixed_point
    take_absolute_value = false
    prop_name = ucopy_change
  []
[]

[Postprocessors]
  [u_change_max]
    type = ElementExtremeFunctorValue
    functor = u_change
    value_type = max
    execute_on = 'MULTISYSTEM_FIXED_POINT_ITERATION_END'
  []
  [ucopy_change_max]
    type = ElementExtremeFunctorValue
    functor = ucopy_change
    value_type = max
    force_postaux = true
    execute_on = 'MULTISYSTEM_FIXED_POINT_ITERATION_END'
  []
[]

[Convergence]
  [u_change_conv]
    type = MultiPostprocessorConvergence
    postprocessors = 'u_change_max ucopy_change_max'
    tolerances = '1e-6 1e-6'
    max_iterations = 10
    verbose = true
  []
[]

[Executioner]
  multi_system_fixed_point_convergence := u_change_conv
[]

[Outputs]
  file_base := multisystem_fp
[]
