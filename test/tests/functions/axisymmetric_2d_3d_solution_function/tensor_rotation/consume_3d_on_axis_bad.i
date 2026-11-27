# 3D consumer that includes on-axis nodes (x=0, z=0 along y-axis).
# Source has T_rr != T_tt so the on-axis guard fires a mooseError.

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
    mesh = 'source_2d_on_axis_bad_out.e'
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
[]

[AuxVariables]
  [T_xx]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [aux_xx]
    type = FunctionAux
    variable = T_xx
    function = f_xx
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
