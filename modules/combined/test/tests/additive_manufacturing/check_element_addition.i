[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
    zmin = 0
    zmax = 0.5
    nx = 20
    ny = 20
    nz = 1
  []
  [left_domain]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '5 10 0.5'
    block_id = 1
  []
  [right_domain]
    input = left_domain
    type = SubdomainBoundingBoxGenerator
    bottom_left = '5 0 0'
    top_right = '10 10 0.5'
    block_id = 2
  []
  [sidesets]
    input = right_domain
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
  [fx]
    type = ParsedFunction
    expression = '5.25'
  []
  [fy]
    type = ParsedFunction
    expression = '2.5*t'
  []
  [fz]
    type = ParsedFunction
    expression = '0.25'
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
  end_time = 1.0
  dt = 1e-1
  dtmin = 1e-4
[]

[UserObjects]
  [activated_elem_uo]
    type = ActivateElementsByPath
    execute_on = timestep_begin
    function_x = fx
    function_y = fy
    function_z = fz
    active_subdomain_id = 1
    expand_boundary_name = 'moving_interface'
  []
[]

[Outputs]
  exodus = true
[]
