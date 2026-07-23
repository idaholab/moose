# Mixed material-input sources feeding one NEML2 interface model.
#
# 'jump' comes from an InterfaceMaterial. 'scale' comes from block material
# data read on the interface sides.
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
  # True interface material.
  [jump]
    type = JumpInterfaceMaterial
    boundary = 'A_B'
    var = u
    neighbor_var = u
  []
  # Spatially varying block material.
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
    # Only 'jump' is from a true InterfaceMaterial.
    interface_material_inputs = 'jump'
  []
[]

[InterfaceKernels]
  [cohesive]
    type = SimpleCohesiveFlux
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
