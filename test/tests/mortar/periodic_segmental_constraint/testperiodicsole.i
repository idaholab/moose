[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 1.0
    ymin = -1.0
    ymax = 1.0
    nx = 2
    ny = 2
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
  [left]
    type = LowerDBlockFromSidesetGenerator
    input = left_block_id
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
  [kappa_x]
    order = FIRST
    family = SCALAR
  []
  [kappa_y]
    order = FIRST
    family = SCALAR
  []
[]

[AuxVariables]
  [kappa_aux]
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
  [kappa]
    type = FunctionScalarAux
    variable = kappa_aux
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
    block = 1
  [../]
  [./flux_y]
    type = DiffusionFluxAux
    diffusivity = 'conductivity'
    variable = flux_y
    diffusion_variable = u
    component = y
    block = 1
  [../]
[]

[Kernels]
  [diff1]
    type = Diffusion
    variable = u
    block = 1
  []
[]

[Materials]
  [k1]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 1.0
    block = 1
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
    penalty_value = 1.e3
  []
  [periodiclrx]
    type = TestPeriodicSole
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    kappa = kappa_x
    kappa_aux = kappa_aux
    component = 0
    kappa_other = kappa_y
    correct_edge_dropping = true
    penalty_value = 1.e3
  []
  [periodiclry]
    type = TestPeriodicSole
    primary_boundary = '11'
    secondary_boundary = '13'
    primary_subdomain = 'primary_right'
    secondary_subdomain = 'secondary_left'
    secondary_variable = u
    kappa = kappa_y
    kappa_aux = kappa_aux
    component = 1
    kappa_other = kappa_x
    correct_edge_dropping = true
    penalty_value = 1.e3
  []
  [mortarbt]
    type = PenaltyEqualValueConstraint
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    correct_edge_dropping = true
    penalty_value = 1.e3
  []
  [periodicbtx]
    type = TestPeriodicSole
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    kappa = kappa_x
    kappa_aux = kappa_aux
    component = 0
    kappa_other = kappa_y
    correct_edge_dropping = true
    penalty_value = 1.e3
  []
  [periodicbty]
    type = TestPeriodicSole
    primary_boundary = '12'
    secondary_boundary = '10'
    primary_subdomain = 'primary_top'
    secondary_subdomain = 'secondary_bottom'
    secondary_variable = u
    kappa = kappa_y
    kappa_aux = kappa_aux
    component = 1
    kappa_other = kappa_x
    correct_edge_dropping = true
    compute_scalar_residuals = true
    penalty_value = 1.e3
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
