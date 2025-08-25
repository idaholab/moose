[Mesh]
  [file]
    type = FileMeshGenerator
    file = 'gold/2blk-conf.msh'
  []
  [secondary]
    type = LowerDBlockFromSidesetGenerator
    input = file
    sidesets = 'lower_half_internal_boundary'
    new_block_id = '101'
    new_block_name = 'secondary_lower'
  []
  [primary]
    type = LowerDBlockFromSidesetGenerator
    input = secondary
    sidesets = 'upper_half_internal_boundary'
    new_block_id = '102'
    new_block_name = 'primary_lower'
  []
  patch_update_strategy = ITERATION
  # for consistent CSV output
  allow_renumbering = false
  second_order = true
[]

[GlobalParams]
  use_displaced_mesh = false
  displacements = 'disp_x disp_y'
[]

# Pre-declare future subdomain
[Mesh]
  add_subdomain_names = 'null null_lower'
  add_subdomain_ids = '3 103'
[]

[Problem]
  kernel_coverage_check = false
[]

[MeshModifiers]
  # Change the subdomains on every time step, starting from the bottom
  # See 'entering_from_left' for the variable guiding the subdomain changes
  [deactivate_regular_elems]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'entering_from_left'
    criterion_type = 'ABOVE'
    threshold = 0.5
    block = '1 2'
    # subdomain 3 is inactive, no variables defined on it
    subdomain_id = 3
    moving_boundary_subdomain_pairs = '2 1; 1 2; 1; 2'
    moving_boundaries = 'upper_half_internal_boundary lower_half_internal_boundary lower_half_external_boundary upper_half_external_boundary'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    execution_order_group = '0'
  []
  [deactivate_lowerD_elems]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'entering_from_left'
    criterion_type = 'ABOVE'
    threshold = 0.5
    block = '101 102'
    # subdomain 103 is inactive, no variables defined on it
    subdomain_id = 103
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
    block = 'lower_half upper_half'
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
    variable = 'u'
    block = 'lower_half upper_half'
  []
  [ffn]
    type = BodyForce
    variable = 'u'
    function = 'ffn'
    block = 'lower_half upper_half'
  []
[]

[Constraints]
  [equal]
    type = EqualValueConstraint
    variable = 'lambda'
    secondary_variable = 'u'
    primary_boundary = 'upper_half_internal_boundary'
    primary_subdomain = 'primary_lower'
    secondary_boundary = 'lower_half_internal_boundary'
    secondary_subdomain = 'secondary_lower'
  []
[]

[AuxVariables]
  [entering_from_left]
    order = CONSTANT
    family = MONOMIAL
    [AuxKernel]
      type = ParsedAux
      expression = 'if(t > x * 4, 1, 0)'
      use_xyzt = true
      # both full-dimensional and low-dimensional should change subdomains
      block = 'lower_half upper_half secondary_lower primary_lower'
      execute_on = 'INITIAL TIMESTEP_BEGIN'
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
    variable = 'u'
    boundary = 'lower_half_external_boundary upper_half_external_boundary'
    function = 'exact_sln'
  []
[]

[Postprocessors]
  [l2_error]
    type = ElementL2Error
    variable = 'u'
    function = 'exact_sln'
    block = 'lower_half upper_half'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_lower_half]
    type = VolumePostprocessor
    block = 'lower_half'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_upper_half]
    type = VolumePostprocessor
    block = 'upper_half'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [vol_null]
    type = VolumePostprocessor
    block = 'null'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [fmp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu NONZERO'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-11

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
    block = 'lower_half upper_half'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
