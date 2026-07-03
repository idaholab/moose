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
  # Block material used as the NEML2 input.
  [u]
    type = GenericConstantMaterial
    prop_names = 'u'
    prop_values = 0.5
    block = 'A B'
  []
[]

[InterfaceKernels]
  [source]
    type = SimpleCohesiveFlux
    variable = u
    neighbor_var = u
    boundary = 'A_B'
    flux = 's'
    dflux_djump = 'ds/du'
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
    interface_only = true
  []
[]

[Executioner]
  type = Steady
  # PJFNK (matrix-free): the NEML2 input is a block material, so the flux is not a function of the
  # FE jump. SimpleCohesiveFlux therefore cannot form an exact Jacobian; the matrix-free operator
  # supplies the true Jacobian action while the assembled matrix serves as the preconditioner.
  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-08
[]

[Outputs]
  exodus = true
[]
