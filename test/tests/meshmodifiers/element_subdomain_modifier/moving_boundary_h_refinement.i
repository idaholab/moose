
[Problem]
  kernel_coverage_check = false
  boundary_restricted_node_integrity_check = false
  boundary_restricted_elem_integrity_check = false
  type = FEProblem
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 4e-3
    ymax = 4e-3
  []
  [active_domain] #the initially active domain
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '4e-3 800e-6 0'
  []
  [inactive_domain] #the initially inactive domain
    type = SubdomainBoundingBoxGenerator
    input = active_domain
    block_id = 2
    bottom_left = '0 800e-6 0'
    top_right = '4e-3 4e-3 0'
  []
  [sideset_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = inactive_domain
    new_boundary = 'sideset_top'
    paired_block = 2
    primary_block = 1
  []
  [sideset_top_other_side]
    type = SideSetsBetweenSubdomainsGenerator
    input = sideset_top
    new_boundary = 'sideset_top_other_side'
    paired_block = 1
    primary_block = 2
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [layerY]
    [AuxKernel]
      type = ParsedAux
      expression = 'y - 50e-6 * ceil(t/1.000001)'
      use_xyzt = true
      execute_on = 'INITIAL TIMESTEP_BEGIN'
    []
  []
[]

[MeshModifiers]
  [addLayer]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'layerY'
    criterion_type = BELOW
    threshold = 0
    subdomain_id = 1
    moving_boundaries = 'sideset_top sideset_top_other_side'
    moving_boundary_subdomain_pairs = '1 2; 2 1'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  end_time = 80
  dt = 1
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  csv = true
[]

[Adaptivity]
  [Markers]
    [boundary_moving]
      type = BoundaryMarker
      next_to = 'sideset_top'
      mark = refine
    []
  []
  max_h_level = 3
  marker = boundary_moving
[]

[Postprocessors]
  [area_top]
    type = AreaPostprocessor
    boundary = 'sideset_top'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [area_top_other]
    type = AreaPostprocessor
    boundary = 'sideset_top'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
