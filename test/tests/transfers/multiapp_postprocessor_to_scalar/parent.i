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

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = u
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  num_steps = 5
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [pp_sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0.5 0.5 0 0.7 0.7 0'
    execute_on = timestep_end
    input_files = sub.i
  []
[]

[Transfers]
  [pp_transfer]
    type = MultiAppPostprocessorToAuxScalarTransfer
    to_multi_app = pp_sub
    from_postprocessor = average
    to_aux_scalar = from_parent_app
  []
[]
