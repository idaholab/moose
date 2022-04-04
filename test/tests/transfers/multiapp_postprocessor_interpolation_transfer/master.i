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

[AuxVariables]
  [from_sub]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
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
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    positions = '0.2 0.2 0 0.7 0.7 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'sub0.i sub1.i'
  []
[]

[Transfers]
  [pp_transfer]
    postprocessor = average
    variable = from_sub
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = sub
  []
[]
