# 3D consumer: non-default axis (z-axis, i.e. 3d_axis_point2='0 0 1').
# Same source as default_axis test; axis override exercises the general-axis
# code path in Axisymmetric2D3DSolutionFunction::value.
# Mesh avoids the z-axis: xmin=1, ymin=1.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    xmin = 1
    xmax = 4
    ny = 4
    ymin = 1
    ymax = 4
    nz = 4
    zmin = 0
    zmax = 4
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
    initial_condition = 0
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [back]
    type = DirichletBC
    variable = u
    boundary = back
    value = 0
  []
[]

[UserObjects]
  [sol]
    type = SolutionUserObject
    mesh = 'source_2d_out.e'
    system_variables = 'T_rr T_yy T_ry T_tt'
    timestep = LATEST
    force_replicated_source_mesh = true
  []
[]

[Functions]
  [f_xx]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 0
    component_j = 0
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
  [f_yy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 1
    component_j = 1
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
  [f_zz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 2
    component_j = 2
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
  [f_xy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 0
    component_j = 1
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
  [f_xz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 0
    component_j = 2
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
  [f_yz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 1
    component_j = 2
    3d_axis_point1 = '0 0 0'
    3d_axis_point2 = '0 0 1'
  []
[]

[AuxVariables]
  [T_xx]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_yy_cart]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_zz]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_xy]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_xz]
    family = MONOMIAL
    order = CONSTANT
  []
  [T_yz]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [aux_xx]
    type = FunctionAux
    variable = T_xx
    function = f_xx
    execute_on = INITIAL
  []
  [aux_yy]
    type = FunctionAux
    variable = T_yy_cart
    function = f_yy
    execute_on = INITIAL
  []
  [aux_zz]
    type = FunctionAux
    variable = T_zz
    function = f_zz
    execute_on = INITIAL
  []
  [aux_xy]
    type = FunctionAux
    variable = T_xy
    function = f_xy
    execute_on = INITIAL
  []
  [aux_xz]
    type = FunctionAux
    variable = T_xz
    function = f_xz
    execute_on = INITIAL
  []
  [aux_yz]
    type = FunctionAux
    variable = T_yz
    function = f_yz
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
