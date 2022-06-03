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

[ICs]
  [ic]
    type = ScalarComponentIC
    variable = a
    values = '1.0 2.0 3.0 4.0 5.0 6.0'
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
  [to_sub]
    type = MultiAppScalarToAuxScalarTransfer
    to_multi_app = sub
    source_variable = 'a'
    to_aux_scalar = 'b'
  []
[]

[Outputs]
    exodus = true
[]
