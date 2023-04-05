[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 1.0
    ymin = -1.0
    ymax = 1.0
    nx = 16
    ny = 16
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [./lowrig]
    type = SubdomainBoundingBoxGenerator
    input = 'left_block_id'
    block_id = 2
    bottom_left = '0 -1 0'
    top_right = '1 0 0'
  [../]
  [./upplef]
    type = SubdomainBoundingBoxGenerator
    input = 'lowrig'
    block_id = 3
    bottom_left = '-1 0 0'
    top_right = '0 1 0'
  [../]
  [./upprig]
    type = SubdomainBoundingBoxGenerator
    input = 'upplef'
    block_id = 4
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  [../]
  [left]
    type = LowerDBlockFromSidesetGenerator
    input = upprig
    sidesets = '13'
    new_block_id = '10003'
    new_block_name = 'secondary_left'
  []
  [right]
    type = LowerDBlockFromSidesetGenerator
    input = left
    sidesets = '11'
    new_block_id = '10001'
    new_block_name = 'primary_right'
  []
  [bottom]
    type = LowerDBlockFromSidesetGenerator
    input = right
    sidesets = '10'
    new_block_id = '10000'
    new_block_name = 'secondary_bottom'
  []
  [top]
    type = LowerDBlockFromSidesetGenerator
    input = bottom
    sidesets = '12'
    new_block_id = '10002'
    new_block_name = 'primary_top'
  []

  [corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = top
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [epsilon]
    order = SECOND
    family = SCALAR
  []
[]

[AuxVariables]
  [sigma]
    order = SECOND
    family = SCALAR
  []
  [./flux_x]
      order = FIRST
      family = MONOMIAL
  [../]
  [./flux_y]
      order = FIRST
      family = MONOMIAL
  [../]
[]

[AuxScalarKernels]
  [sigma]
    type = FunctionScalarAux
    variable = sigma
    function = '1 3'
    execute_on = initial #timestep_end
  []
[]

[AuxKernels]
  [./flux_x]
    type = DiffusionFluxAux
    diffusivity = 'conductivity'
    variable = flux_x
    diffusion_variable = u
    component = x
    block = '1 2 3 4'
  [../]
  [./flux_y]
    type = DiffusionFluxAux
    diffusivity = 'conductivity'
    variable = flux_y
    diffusion_variable = u
    component = y
    block = '1 2 3 4'
  [../]
[]

[Kernels]
  [diff1]
    type = Diffusion
    variable = u
    block = '1 4'
  []
  [diff2]
    type = MatDiffusion
    variable = u
    block = '2 3'
    diffusivity = conductivity
  []
[]

[Materials]
  [k1]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 1.0
    block = '1 4'
  []
  [k2]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 10.0
    block = '2 3'
  []
[]

[Problem]
  kernel_coverage_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [fix_right]
    type = DirichletBC
    variable = u
    boundary = pinned_node
    value = 0
  []
[]

[Constraints]
  [mortarlr]
    type = PenaltyEqualValueConstraint
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodiclr]
    type = PenaltyPeriodicSegmentalConstraint
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [mortarbt]
    type = PenaltyEqualValueConstraint
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
  [periodicbt]
    type = PenaltyPeriodicSegmentalConstraint
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    epsilon = epsilon
    sigma = sigma
    correct_edge_dropping = true
    penalty_value = 1.e2
  []
[]

[Preconditioning]
  [smp]
    full = true
    type = SMP
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
[]

[Postprocessors]
  [max]
    type = ElementExtremeValue
    variable = 'flux_x'
  []
[]

[Outputs]
  csv = true
[]
