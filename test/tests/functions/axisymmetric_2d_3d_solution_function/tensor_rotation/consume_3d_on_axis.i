# 3D consumer that includes on-axis nodes (x=0, z=0 along y-axis).
# Source satisfies T_rr=T_tt, T_ry=0 so the on-axis guard passes and
# the function returns diag(T_rr, T_yy, T_rr) at r=0 nodes.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    xmin = 0
    xmax = 4
    ny = 4
    ymin = 0
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
    mesh = 'source_2d_on_axis_out.e'
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
  []
  [f_yy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 1
    component_j = 1
  []
  [f_zz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 2
    component_j = 2
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
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
