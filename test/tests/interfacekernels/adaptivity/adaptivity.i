# This input file is used for two tests:
# 1) Check that InterfaceKernels work with mesh adaptivity
# 2) Error out when InterfaceKernels are used with adaptivity
#    and stateful material prpoerties

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  parallel_type = 'replicated'
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./break_boundary]
    depends_on = interface
    type = BreakBoundaryOnSubdomain
  [../]
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
    block = 0
  [../]
  [./u_neighbor]
    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
    block = 1
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = (x*x*x)-6.0*x
  [../]

  [./bc_fn]
    type = ParsedFunction
    value = (x*x*x)
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    variable = u
    prop_name = diffusivity
    block = 0
  [../]
  [./abs]
    type = Reaction
    variable = u
    block = 0
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
    block = 0
  [../]
  [./diffn]
    type = MatDiffusion
    variable = u_neighbor
    prop_name = diffusivity
    block = 1
  [../]
  [./absn]
    type = Reaction
    variable = u_neighbor
    block = 1
  [../]
  [./forcingn]
    type = BodyForce
    variable = u_neighbor
    function = forcing_fn
    block = 1
  [../]
[]

[InterfaceKernels]
  [./flux_match]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = u_neighbor
    boundary = master0_interface
    D = 1
    D_neighbor = 1
  [../]
[]

[BCs]
  [./u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left'
    function = bc_fn
  [../]
  [./u_neighbor]
    type = FunctionDirichletBC
    variable = u_neighbor
    boundary = 'right'
    function = bc_fn
  [../]
  [./interface]
    type = MatchedValueBC
    variable = u_neighbor
    v = u
    boundary = 'master0_interface'
  [../]
[]

[Materials]
  active = 'constant'
  [./stateful]
    type = StatefulTest
    prop_names = 'diffusivity'
    prop_values = '1'
    block = '0 1'
  [../]
  [./constant]
    type = GenericConstantMaterial
    prop_names = 'diffusivity'
    prop_values = '1'
    block = '0 1'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Adaptivity]
  marker = 'marker'
  steps = 1
  [./Markers]
    [./marker]
      type = BoxMarker
      bottom_left = '0 0 0'
      top_right = '1 1 0'
      inside = refine
      outside = coarsen
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
