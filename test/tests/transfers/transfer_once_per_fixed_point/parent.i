[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  type = FEProblem
  solve = false
  verbose_multiapps = true
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  fixed_point_min_its = 4
  fixed_point_max_its = 20

  verbose = true
[]

[MultiApps]
  # This app is used to trigger fixed point iteration when sub is executed on MULTIAPP_FIXED_POINT_BEGIN/END
  [side_app]
    type = TransientMultiApp
    input_files = sub.i
    cli_args = "MultiApps/active='';Outputs/active=''"
    execute_on = 'INITIAL TIMESTEP_END'
    # The input was originally created with effectively no restore
    # see the changes made for #5554 then reverted in #28115
    no_restore = true
  []
  # This app is used to test the fixed point begin/end execute_on for transfers and multiapps
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = 'INITIAL TIMESTEP_END'
    # The input was originally created with effectively no restore
    # see the changes made for #5554 then reverted in #28115
    no_restore = true
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = num_fixed_point_total
    to_postprocessor = parent_fp_its
    to_multi_app = sub
    check_multiapp_execute_on = false
    execute_on = 'MULTIAPP_FIXED_POINT_BEGIN'
  []
  [from_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = num_fixed_point_its
    to_postprocessor = subapp_fp_its
    from_multi_app = sub
    check_multiapp_execute_on = false
    execute_on = 'MULTIAPP_FIXED_POINT_BEGIN'
    reduction_type = 'sum'
  []
[]

[Postprocessors]
  [num_fixed_point_total]
    type = TestPostprocessor
    test_type = 'grow'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [num_fixed_point_begin]
    type = TestPostprocessor
    test_type = 'grow'
    execute_on = 'MULTIAPP_FIXED_POINT_BEGIN'
  []
  [num_fixed_point_end]
    type = TestPostprocessor
    test_type = 'grow'
    execute_on = 'MULTIAPP_FIXED_POINT_END'
  []
  [subapp_fp_its]
    type = Receiver
  []
[]

[Outputs]
  [fp_begin]
    type = CSV
    execute_on = 'MULTIAPP_FIXED_POINT_BEGIN'
  []
  [fp_end]
    type = CSV
    file_base = 'fp_end'
    execute_on = 'MULTIAPP_FIXED_POINT_END'
  []
[]
