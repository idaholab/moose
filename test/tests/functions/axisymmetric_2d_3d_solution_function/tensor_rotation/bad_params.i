# Base input for bad-parameter RunException tests.
# Default state: scalar mode (1 from_variable), no component params.
# Each test case overrides parameters via cli_args to trigger one error path.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    xmin = 1
    xmax = 3
    ny = 2
    ymin = 0
    ymax = 2
    nz = 2
    zmin = 1
    zmax = 3
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
  [f]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr'
  []
[]

[AuxVariables]
  [val]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [aux_val]
    type = FunctionAux
    variable = val
    function = f
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
[]
