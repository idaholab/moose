[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [force_u]
    type = CoupledForce
    variable = u
    v = v
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [unorm_begin]
    type = ElementL2Norm
    variable = u
    execute_on = 'initial multiapp_fixed_point_begin'
    outputs = none
  []
  [unorm]
    type = ElementL2Norm
    variable = u
    execute_on = 'initial multiapp_fixed_point_end'
  []
  [unorm_err]
    type = RelativeDifferencePostprocessor
    value1 = unorm
    value2 = unorm_begin
    outputs = none
    execute_on = 'multiapp_fixed_point_end'
  []
  [vnorm]
    type = ElementL2Norm
    variable = v
    execute_on = 'initial multiapp_fixed_point_end'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  disable_fixed_point_residual_norm_check = true
  custom_pp = unorm_err
  nl_abs_tol = 1e-14
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = steady_picard_sub.i
    no_restore = true
  []
[]

[Transfers]
  [v_from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
  []
  [u_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    variable = u
  []
[]
