[Mesh]
  [file]
    type = FileMeshGenerator
    file = '../continuity-2d-conforming/2blk-conf.e'
  []
  [rename]
    type = RenameBlockGenerator
    input = file
    old_block = '1 2'
    new_block = '0 1'
  []
  [rename_bdy]
    type = RenameBoundaryGenerator
    new_boundary = 'left top right bottom'
    old_boundary = '1 2 3 4'
    input = rename
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = rename_bdy
    sidesets = '101'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = '100'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
  [separate_elements]
    type = MeshRepairGenerator
    input = primary
    separate_blocks_by_element_types = true
  []
  patch_update_strategy = ITERATION
  # for consistent CSV output
  allow_renumbering = false
  second_order = true
[]

# Turn on displaced mesh everywhere
[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y'
[]

# Pre-declare future subdomain
[Mesh]
  add_subdomain_ids = '2 10002'
[]
[Problem]
  kernel_coverage_check = false
[]

[MeshModifiers]
  # Change the subdomains on every time step, starting from the bottom
  # See 'rising_from_bottom' for the variable guiding the subdomain changes
  [inactivate_regular_elems_from_the_bottom]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'rising_from_bottom'
    criterion_type = 'ABOVE'
    threshold = 0.5
    block = '0 1'
    # subdomain 2 is inactive, no variables defined on it
    subdomain_id = 2
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    execution_order_group = '0'
  []
  [inactivate_lowerD_elems_from_the_bottom]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'rising_from_bottom'
    criterion_type = 'ABOVE'
    threshold = 0.5
    block = '10000 10001'
    # subdomain 10002 is inactive, no variables defined on it
    subdomain_id = 10002
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    execution_order_group = '2'
  []
  # Update the sidesets between domain 1 and 2 as we change the mesh
  # Otherwise we would execute the mortar constraints by block 2, where u is not defined
  [update_boundary_100]
    type = SidesetAroundSubdomainUpdater
    update_boundary_name = 100
    inner_subdomains = '1'
    outer_subdomains = '0'
    use_displaced_mesh = true
    assign_outer_surface_sides = false
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    execution_order_group = '1'
  []
  [update_boundary_101]
    type = SidesetAroundSubdomainUpdater
    update_boundary_name = 101
    inner_subdomains = '0'
    outer_subdomains = '1'
    use_displaced_mesh = true
    assign_outer_surface_sides = false
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    execution_order_group = '1'
  []
[]

[Functions]
  [exact_sln]
    type = ParsedFunction
    expression = y
  []
  [ffn]
    type = ParsedFunction
    expression = 0
  []
[]

[Variables]
  [u]
    order = SECOND
    family = LAGRANGE
    block = '0 1'
  []
  [lambda]
    order = FIRST
    family = LAGRANGE
    block = 'secondary_lower'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = ffn
  []
[]

[Constraints]
  [equal]
    type = EqualValueConstraint
    variable = lambda
    secondary_variable = 'u'
    primary_boundary = 100
    primary_subdomain = 10000
    secondary_boundary = 101
    secondary_subdomain = 10001
  []
[]

[AuxVariables]
  [rising_from_bottom]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ParsedAux
      expression = 'if(t > x * 4, 1, 0)'
      use_xyzt = true
      # both full-dimensional and low-dimensional should change subdomains
      block = '0 1 10000 10001'
    []
  []
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'right left top bottom'
    function = exact_sln
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = u
    function = exact_sln
    block = '0 1'
    execute_on = 'initial timestep_end'
  []
  [vol_0]
    type = VolumePostprocessor
    block = 0
  []
  [vol_1]
    type = VolumePostprocessor
    block = 1
  []
  [vol_2]
    type = VolumePostprocessor
    block = 2
  []
[]

[Preconditioning]
  [fmp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
    # Default PC fails at 14 MPI
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu NONZERO'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  nl_abs_tol = 1e-12

  dtmin = 1
[]

# Testing considerations:
# exodus output does not like overlapping elements
# block-restricted exodus would not handle the changing mesh
# csv nodal-sampling gets affected by node-renumbering
# But this works!
[Outputs]
  csv = true
[]

[Positions]
  [functors]
    type = FunctorExtremaPositions
    functor = 'u'
    extrema_type = 'MAX'
    # only 8 nodes on final step
    num_extrema = 8
    block = '0 1'
    execute_on = 'timestep_end'
  []
[]
