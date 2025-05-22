[Mesh]
  add_subdomain_ids = '2'
  add_subdomain_names = 'ghost'

  add_sideset_ids = '4001 4002'
  add_sideset_names = 'right top'

  add_nodeset_ids = '4001 4002'
  add_nodeset_names = 'right top'

  construct_node_list_from_side_list = true

  [block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 4
    nx = 4
    ny = 4
    boundary_id_offset = 0
    subdomain_ids = 1
    subdomain_name = block
  []
  [rename_new_area_sidesets1]
    type = RenameBoundaryGenerator
    input = 'block'
    new_boundary = 'mask_top'
    old_boundary = 'top'
  []
  [rename_new_area_sidesets2]
    type = RenameBoundaryGenerator
    input = 'rename_new_area_sidesets1'
    new_boundary = 'mask_right'
    old_boundary = 'right'
  []
  final_generator = rename_new_area_sidesets2
[]
[MeshModifiers]
  [ghost_parts]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    criterion_type = 'ABOVE'
    threshold = 0
    block = '1'
    subdomain_id = 2
    execute_on = 'INITIAL TIMESTEP_END'
    reinitialize_subdomains = '1 2'
    execution_order_group = 0
  []
  [TOP_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = '1'
    outer_subdomains = '2'
    update_boundary_name = 'top'
    mask_side = 'mask_top'
    execute_on = 'INITIAL TIMESTEP_END'
    execution_order_group = 1
  []
  [RIGHT_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = '1'
    outer_subdomains = '2'
    update_boundary_name = 'right'
    mask_side = 'mask_right'
    execute_on = 'INITIAL TIMESTEP_END'
    execution_order_group = 1
  []
[]
[Controls]
  [control_ghost_cladding]
    type = TimePeriod
    enable_objects = 'AuxKernels/ghost_cladding'
    start_time = 2
    end_time = 3
    execute_on = 'initial timestep_begin'
  []
[]
[AuxVariables]
  [phi]
    initial_condition = 0
    block = 1
  []
[]
[AuxKernels]
  [ghost_cladding]
    type = ParsedAux
    expression = "if(y>1 & y<3, 1, 0)"
    use_xyzt = true
    variable = 'phi'
    enable = false
  []
[]

[Variables]
  [u]
    block = '1'
  []
  [null]
    block = '2'
  []
[]
[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = '1'
  []
  [nothing]
    type = NullKernel
    variable = null
    block = '2'
  []
[]
[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = mask_top
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = mask_right
    value = 1
  []
[]
[Executioner]
  type = Transient
  solve_type = PJFNK
  l_max_its = 100
  l_tol = 1e-4
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-12

  start_time = -1
  dt = 1
  end_time = 9
[]
[Postprocessors]
  [volume1]
    type = VolumePostprocessor
    block = 1
    execute_on = 'initial timestep_end'
  []
  [volume2]
    type = VolumePostprocessor
    block = 2
    execute_on = 'initial timestep_end'
  []
  [top_flux]
    type = SideIntegralVariablePostprocessor
    variable = 'u'
    boundary = '4002'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [right_flux]
    type = SideIntegralVariablePostprocessor
    variable = 'u'
    boundary = '4001'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [top_area]
    type = AreaPostprocessor
    boundary = '4002'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [right_area]
    type = AreaPostprocessor
    boundary = '4001'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [top_nodes]
    type = NodalSum
    variable = u
    boundary = '4002'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [right_nodes]
    type = NodalSum
    variable = u
    boundary = '4001'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
