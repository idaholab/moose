# Mixed material-input sources feeding one NEML2 interface model.
#
# The single [NEML2] action below takes two MATERIAL inputs from DIFFERENT MOOSE material-data
# stores:
#   - 'jump'  from JumpInterfaceMaterial, a true InterfaceMaterial (INTERFACE_MATERIAL_DATA)
#   - 'scale' from a block GenericConstantMaterial evaluated on the interface sides (FACE/NEIGHBOR)
# Only 'jump' is listed in interface_material_inputs. This is the case a single sub-block-global
# flag could not express: a global true would wrongly route 'scale' to interface data, a global
# false would wrongly route 'jump' to volume/side data. Per-input routing keeps both correct.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 2
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
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  []
[]

[Materials]
  # True interface material: supplies 'jump' on the interface itself.
  [jump]
    type = JumpInterfaceMaterial
    boundary = 'A_B'
    var = u
    neighbor_var = u
  []
  # Block material: supplies 'scale' on the volume, read on the interface sides. Made spatially
  # varying so that mis-routing it through interface data (or mis-routing 'jump' through block
  # data) changes the result, not just the code path.
  [scale]
    type = GenericFunctionMaterial
    prop_names = 'scale'
    prop_values = 'scale_fn'
    block = 'A B'
  []
[]

[Functions]
  [scale_fn]
    type = ParsedFunction
    expression = '0.2 + x + y'
  []
[]

[NEML2]
  input = 'models/mixed_cohesive_model.i'
  device = 'cpu'

  input_types = 'MATERIAL MATERIAL'
  inputs = 'jump scale'

  derivatives = 'q jump'

  [cohesive]
    model = 'mixed_cohesive'
    interface = 'A_B'
    interface_only = true
    # Only 'jump' is from a true InterfaceMaterial; 'scale' stays on the block/side source.
    interface_material_inputs = 'jump'
  []
[]

[InterfaceKernels]
  [cohesive]
    type = NEML2CohesiveFlux
    variable = u
    neighbor_var = u
    boundary = 'A_B'
    flux = 'q'
    dflux_djump = 'dq/djump'
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
