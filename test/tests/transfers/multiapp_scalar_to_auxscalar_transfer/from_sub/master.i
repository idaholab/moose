[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [a]
    family = SCALAR
    order = SIXTH
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = dummy
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'sub.i'
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppScalarToAuxScalarTransfer
    from_multi_app = sub
    source_variable = 'b'
    to_aux_scalar = 'a'
  []
[]

[Outputs]
    exodus = true
[]
