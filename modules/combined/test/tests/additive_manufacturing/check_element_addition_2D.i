[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    ymin = 0
    xmax = 1
    ymax = 0.5
    nx = 20
    ny = 10
  []
  [bottom_domain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = ' 1 0.1 0'
    block_id = 1
  []
  [top_domain]
    input = bottom_domain
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0.1 0'
    top_right = '1 0.5 00'
    block_id = 2
  []
  [sidesets]
    input = top_domain
    type = SideSetsAroundSubdomainGenerator
    normal = '1 0 0'
    block = 1
    new_boundary = 'moving_interface'
  []
[]

[Variables]
  [temp]
    block = '1'
  []
[]

[Functions]
  [fy]
    type = ParsedFunction
    expression = '0.2'
  []
  [fx]
    type = ParsedFunction
    expression = 't'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  automatic_scaling = true
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  line_search = 'none'

  l_max_its = 10
  nl_max_its = 20
  nl_rel_tol = 1e-4

  start_time = 0.0
  end_time = 1
  dt = 1e-1
  dtmin = 1e-4
[]

[UserObjects]
  [activated_elem_uo]
    type = ActivateElementsByPath
    execute_on = timestep_begin
    activate_distance = 0.2
    function_x = fx
    function_y = fy
    active_subdomain_id = 1
    expand_boundary_name = 'moving_interface'
  []
[]

[Outputs]
  exodus = true
[]
