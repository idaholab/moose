[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Functions]
  [v_fn]
    type = ParsedFunction
    value = t*x
  []
  [ffn]
    type = ParsedFunction
    value = x
  []
[]

[AuxVariables]
  [v]
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [td]
    type = TimeDerivative
    variable = u
  []
  [ufn]
    type = BodyForce
    variable = u
    function = ffn
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right'
    function = v_fn
  []
[]

[Executioner]
  type = Transient
  num_steps = 7
  dt = 0.1
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
  csv = true
  [out]
    type = Checkpoint
    end_step = 5
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    input_files = 'sub.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppCopyTransfer
    direction = to_multiapp
    multi_app = sub_app
    source_variable = u
    variable = u
  []
  [from_sub]
    type = MultiAppCopyTransfer
    direction = from_multiapp
    multi_app = sub_app
    source_variable = u
    variable = v
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    execute_on = 'timestep_end'
    variable = v
  []
[]
