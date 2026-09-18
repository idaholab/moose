[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 3
  []
  [block_A]
    type = SubdomainBoundingBoxGenerator
    input = 'gmg'
    block_id = 0
    block_name = 'block_A'
    bottom_left = '0 0 0'
    top_right = '0.5 1 0'
  []
  [block_B]
    type = SubdomainBoundingBoxGenerator
    input = 'block_A'
    block_id = 1
    block_name = 'block_B'
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
  []
  [break]
    type = BreakMeshByBlockGenerator
    input = 'block_B'
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

[InterfaceKernels]
  [source]
    type = MaterialPropertySource
    variable = u
    neighbor_var = u
    source = 's'
    dsource_du = 'ds/du'
    dsource_du_neighbor = 0
    boundary = 'block_A_block_B'
  []
[]

[NEML2]
  input = 'models/custom_model.i'
  device = 'cpu'
  eager = true
  load = 'models/test_models.py'

  input_types = 'VARIABLE'
  inputs = 'u'

  derivatives = 's u'

  [source]
    model = 'interface_source'
    block = 'block_A block_B'
    interface = 'block_A_block_B'
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
