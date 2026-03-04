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

[InterfaceKernels]
  [source]
    type = MaterialPropertySource
    variable = u
    neighbor_var = u
    source = 's'
    dsource_du = 'ds/du'
    dsource_du_neighbor = 0
    boundary = 'A_B'
  []
[]

[NEML2]
  input = 'models/custom_model.i'
  model = 'interface_source'
  verbose = true
  device = 'cpu'

  moose_input_types = 'VARIABLE'
  moose_inputs = 'u'
  neml2_inputs = 'state/u'

  moose_output_types = 'MATERIAL'
  moose_outputs = 's'
  neml2_outputs = 'state/s'

  moose_derivative_types = 'MATERIAL'
  moose_derivatives = 'ds/du'
  neml2_derivatives = 'state/s state/u'
  [block]
    block = 'A B'
    interface = 'A_B'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options = '-ksp_view_mat'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
[]

[Outputs]
  exodus = true
[]
