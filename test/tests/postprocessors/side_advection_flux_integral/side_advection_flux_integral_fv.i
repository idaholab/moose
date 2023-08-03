[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.75 0.75 0.75'
    dy = '0.75 0.75 0.75'
    ix = '2 2 2'
    iy = '2 2 2'
    subdomain_id = '1 1 1
                    1 2 1
                    1 1 1'
  []
  [add_inner_boundaries_top]
    type = SideSetsAroundSubdomainGenerator
    input = cmg
    new_boundary = 'block_2_top'
    block = 2
    normal = '0 1 0'
  []
  [add_inner_boundaries_bot]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_top
    new_boundary = 'block_2_bot'
    block = 2
    normal = '0 -1 0'
  []
  [add_inner_boundaries_right]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_bot
    new_boundary = 'block_2_right'
    block = 2
    normal = '1 0 0'
  []
  [add_inner_boundaries_left]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_boundaries_right
    new_boundary = 'block_2_left'
    block = 2
    normal = '-1 0 0'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    two_term_boundary_expansion = false
  []
[]

[ICs]
  [u_blob]
    type = FunctionIC
    variable = u
    function = 'if(x<0.75,if(y<0.75,1,0),0)'
  []
[]

[FVKernels]
  [advection]
    type = FVAdvection
    variable = u
    velocity = '2 1.5 0'
  []
  [time]
    type = FVTimeKernel
    variable = u
  []
[]

[FVBCs]
  [fv_outflow]
    type = FVConstantScalarOutflowBC
    velocity = '2 1.5 0'
    variable = u
    boundary = 'right top'
  []
[]

[Executioner]
  type = Transient
  solve_type = LINEAR
  dt = 0.01
  end_time = 0.02
  l_tol = 1E-14
[]

[Postprocessors]
  [flux_right]
    type = SideAdvectiveFluxIntegral
    boundary = 'block_2_right'
    vel_x = 2
    vel_y = 1.5
    component = x
    advected_quantity = u
  []
  [flux_left_exact]
    type = SideAdvectiveFluxIntegral
    boundary = 'block_2_left'
    vel_x = 2
    vel_y = 1.5
    component = x
    advected_quantity = u
  []
[]

[Outputs]
  csv = true
[]
