# Test block material inputs read from both sides of a NEML2 interface model.
#
# 'u' differs on the two blocks and is not listed in interface_material_inputs.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 3
  []
  [A]
    type = SubdomainBoundingBoxGenerator
    input = 'gmg'
    block_id = 0
    block_name = 'A'
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [B]
    type = SubdomainBoundingBoxGenerator
    input = 'A'
    block_id = 1
    block_name = 'B'
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = 'B'
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
  [react]
    type = Reaction
    variable = u
  []
[]

[Materials]
  # Different values on the two sides of the interface.
  [uA]
    type = GenericConstantMaterial
    prop_names = 'u'
    prop_values = 0.3
    block = 'A'
  []
  [uB]
    type = GenericConstantMaterial
    prop_names = 'u'
    prop_values = 0.7
    block = 'B'
  []
[]

[InterfaceKernels]
  # MaterialPropertySource reads 's' from both interface sides.
  [source]
    type = MaterialPropertySource
    variable = u
    neighbor_var = u
    source = 's'
    dsource_du = 0
    dsource_du_neighbor = 0
    boundary = 'A_B'
  []
[]

[NEML2]
  input = 'models/custom_model.i'
  device = 'cpu'

  input_types = 'MATERIAL'
  inputs = 'u'

  derivatives = 's u'

  [source]
    model = 'interface_source'
    block = 'A B'
    interface = 'A_B'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
[]

[Outputs]
  exodus = true
[]
