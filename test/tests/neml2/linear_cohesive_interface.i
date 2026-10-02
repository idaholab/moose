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
  [jump]
    type = JumpInterfaceMaterial
    boundary = 'A_B'
    var = u
    neighbor_var = u
  []
[]

[NEML2]
  input = 'models/linear_cohesive_model.i'
  device = 'cpu'

  input_types = 'MATERIAL'
  inputs = 'jump'

  derivatives = 'q jump'

  [cohesive]
    model = 'linear_cohesive'
    interface = 'A_B'
    interface_only = true
    # 'jump' is supplied by a true InterfaceMaterial.
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
