num_layers = 2

[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    nx = ${num_layers}
    ny = 3
    nz = 3
  []
  # The MultiAppUserObjectTransfer object only works with ReplicatedMesh
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [td]
    type = TimeDerivative
    variable = u
  []

  [diff]
    type = Diffusion
    variable = u
  []
[]

[AuxVariables]
  [a]
    family = MONOMIAL
    order = CONSTANT
  []

  [s]
  []
[]

[AuxKernels]
  [s_ak]
    type = ParsedAux
    variable = s
    use_xyzt = true
    expression = 'x+(z*z)'
  []
[]

[Functions]
[]

[Postprocessors]
  [a_avg]
    type = ElementAverageValue
    variable = a
  []
[]

[UserObjects]
  [S_avg_front]
    type = LayeredSideAverage
    boundary = front
    variable = s
    num_layers = ${num_layers}
    direction = x
  []

  [S_avg_back]
    type = LayeredSideAverage
    boundary = back
    variable = s
    num_layers = ${num_layers}
    direction = x
  []
[]

[MultiApps]
  [ch0]
    type = TransientMultiApp
    input_files = 'restricted_elem_sub.i'
    bounding_box_padding = '0 0.5 1'
    positions = '0 0.5 -0.1'
    output_in_position = true
    cli_args = 'yy=0'
  []
  [ch1]
    type = TransientMultiApp
    input_files = 'restricted_elem_sub.i'
    bounding_box_padding = '0 0.5 1'
    positions = '0 0.5  1.1'
    output_in_position = true
    cli_args = 'yy=1'
  []
[]

[Transfers]
  [from_ch0]
    type = MultiAppUserObjectTransfer
    boundary = back
    from_multi_app = ch0
    variable = a
    user_object = A_avg
  []

  [from_ch1]
    type = MultiAppUserObjectTransfer
    boundary = front
    from_multi_app = ch1
    variable = a
    user_object = A_avg
  []

  [to_ch0]
    type = MultiAppUserObjectTransfer
    block = 20
    to_multi_app = ch0
    variable = S
    user_object = S_avg_back
  []

  [to_ch1]
    type = MultiAppUserObjectTransfer
    block = 20
    to_multi_app = ch1
    variable = S
    user_object = S_avg_front
  []
[]

[Executioner]
  type = Transient
  num_steps = 2

  dt = 1
  nl_abs_tol = 1e-7
[]

[Outputs]
  exodus = true
[]
