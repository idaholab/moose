[Mesh]
  add_subdomain_names = 'fake'
  add_subdomain_ids = 9999
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 10
    nx = 1
    ny = 10
  []
  [block_1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 10 0'
    input = gen
  []
[]

[MeshModifiers]
  [remove_parts]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    criterion_type = 'ABOVE'
    threshold = 0
    block = '1'
    subdomain_id = 9999
    execute_on = 'TIMESTEP_END'
    reinitialize_subdomains = '1 9999'
    execution_order_group = 6
  []
  [fake_right_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 1
    outer_subdomains = 9999
    update_boundary_name = 'right'
    execute_on = TIMESTEP_END
    execution_order_group = 9
  []
  [fake_left_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 1
    outer_subdomains = 9999
    update_boundary_name = 'left'
    execute_on = TIMESTEP_END
    execution_order_group = 8
  []
  [fake_top_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 1
    outer_subdomains = 9999
    update_boundary_name = 'top'
    execute_on = TIMESTEP_END
    execution_order_group = 7
  []
[]

[Controls]
  [period1]
    type = TimePeriod
    disable_objects = 'AuxKernels/kill'
    start_time = 0
    end_time = 1
    execute_on = 'timestep_begin'
  []
[]

[Variables]
  [u]
    block = 1
  []
  [v]
    block = 9999
  []
[]

[AuxVariables]
  [phi]
    initial_condition = 0
    block = '1'
  []
[]

[AuxKernels]
  [kill]
    type = ParsedAux
    expression = "if(y>8, 1, 0)"
    use_xyzt = true
    variable = 'phi'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    block = '1'
  []
  [fake]
    type = NullKernel
    variable = v
    block = '9999'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 1
  end_time = 2
[]

[Postprocessors]
  [volume1]
    type = VolumePostprocessor
    block = 1
    execute_on = 'initial timestep_end'
  []
  [volume9999]
    type = VolumePostprocessor
    block = 9999
    execute_on = 'initial timestep_end'
  []
  [flux]
    type = AreaPostprocessor
    boundary = 'right'
    execute_on = 'TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
