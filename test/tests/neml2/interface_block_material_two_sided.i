# A block material feeding a NEML2 interface model must be read INDEPENDENTLY on the two sides of
# the interface: the element side from FACE material data and the neighbor side from NEIGHBOR
# material data. The block material 'u' is given a DIFFERENT constant on each subdomain (0.3 on A,
# 0.7 on B), so the NEML2 output s = u*u - 0.1 differs between the two sides. MaterialPropertySource
# consumes the output on BOTH sides (getMaterialProperty on the element side and
# getNeighborMaterialProperty on the neighbor side), so if the two sides were collapsed to a single
# store the solution would change. The gold file therefore locks in the two-sided behavior.
#
# 'u' is a block material (not a true InterfaceMaterial), so it is NOT listed in
# 'interface_material_inputs'.
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
  # Block material with a DIFFERENT value on each side of the interface.
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
  # Reads the NEML2 output 's' on BOTH sides: getMaterialProperty (element) and
  # getNeighborMaterialProperty (neighbor). The source does not depend on the FE variable (the
  # NEML2 input is a block material), so both jump derivatives are zero.
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
