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
  []
  [v]
  []
[]

[ICs]
  [u_blob]
    type = FunctionIC
    variable = u
    function = 'if(x<0.75,if(y<0.75,1,0),0)'
  []

  [v_blob]
    type = FunctionIC
    variable = v
    function = 'if(x<0.75,if(y<0.75,1,0),0)'
  []
[]

[Kernels]
  [udot]
    type = MassLumpedTimeDerivative
    variable = u
  []
  [u_advec]
    type = ConservativeAdvection
    variable = u
    upwinding_type = full
    velocity = '2 0 0'
  []

  [vdot]
    type = MassLumpedTimeDerivative
    variable = v
  []
  [v_advec]
    type = ConservativeAdvection
    variable = v
    upwinding_type = full
    velocity = '0 2 0'
  []
[]

[Materials]
  [rho]
    type = GenericConstantMaterial
    prop_names = 'rho'
    prop_values = '1'
  []
[]

[AuxVariables]
  [flux_x]
    order = FIRST
    family = MONOMIAL
  []
[]

[AuxKernels]
  [flux_x]
    type = AdvectiveFluxAux
    variable = flux_x
    vel_x = u
    vel_y = v
    advected_mat_prop = 'rho'
    component = normal
    boundary = 'block_2_right block_2_left'
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
    type = SideIntegralVariablePostprocessor
    variable = flux_x
    boundary = 'block_2_right'
  []
  [flux_right_exact]
    type = SideAdvectiveFluxIntegral
    boundary = 'block_2_right'
    vel_x = u
    vel_y = v
    component = normal
    advected_mat_prop = 'rho'
  []
  [flux_left]
    type = SideIntegralVariablePostprocessor
    variable = flux_x
    boundary = 'block_2_left'
  []
  [flux_left_exact]
    type = SideAdvectiveFluxIntegral
    boundary = 'block_2_left'
    vel_x = u
    vel_y = v
    component = normal
    advected_mat_prop = 'rho'
  []
[]

[Outputs]
   csv = true
[]
