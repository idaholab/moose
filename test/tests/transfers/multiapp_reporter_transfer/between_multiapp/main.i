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
    type = MultiAppReporterTransfer
    from_multi_app = pp_sub_0
    to_multi_app = pp_sub_1
    from_reporters = 'base_sub0_vpp/a base_sub0_vpp/b'
    to_reporters = 'from_sub0_vpp/a from_sub0_vpp/b'
  []
  [pp_transfer_2]
    type = MultiAppReporterTransfer
    from_multi_app = pp_sub_1
    to_multi_app = pp_sub_0
    from_reporters = 'base_sub1_vpp/a base_sub1_vpp/b'
    to_reporters = 'from_sub1_vpp/a from_sub1_vpp/b'
  []
[]
