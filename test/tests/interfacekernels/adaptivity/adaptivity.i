# This input file is used for two tests:
# 1) Check that InterfaceKernels work with mesh adaptivity
# 2) Error out when InterfaceKernels are used with adaptivity
#    and stateful material prpoerties

[Mesh]
  parallel_type = 'replicated'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./break_boundary]
    input = interface
    type = BreakBoundaryOnSubdomainGenerator
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
    expression = (x*x*x)-6.0*x
  [../]

  [./bc_fn]
    type = ParsedFunction
    expression = (x*x*x)
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
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
    type = MatDiffusionTest
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
    type = PenaltyInterfaceDiffusion
    variable = u
    neighbor_var = u_neighbor
    boundary = primary0_interface
    penalty = 1e6
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

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
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
