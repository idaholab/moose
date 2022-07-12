[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.01
  []
  [td]
    type = TimeDerivative
    variable = u
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

[Executioner]
  type = Transient
  num_steps = 2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-12
[]

[MultiApps]
  [pp_sub_0]
    app_type = MooseTestApp
    positions = '0.5 0.5 0 0.7 0.7 0'
    execute_on = timestep_end
    type = TransientMultiApp
    input_files = sub0.i
  []
  [pp_sub_1]
    app_type = MooseTestApp
    positions = '0.5 0.5 0 0.7 0.7 0'
    execute_on = timestep_end
    type = TransientMultiApp
    input_files = sub1.i
  []
[]

[Transfers]
  [pp_transfer_1]
    type = MultiAppPostprocessorToAuxScalarTransfer
    from_multi_app = pp_sub_0
    to_multi_app = pp_sub_1
    from_postprocessor = average_0
    to_aux_scalar = from_0
  []
  [pp_transfer_2]
    type = MultiAppPostprocessorToAuxScalarTransfer
    from_multi_app = pp_sub_1
    to_multi_app = pp_sub_0
    from_postprocessor = average_1
    to_aux_scalar = from_1
  []
[]
