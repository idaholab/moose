# 3D consumer of source_2d_varying.i.
#
# Source field: T_rr = r, T_yy = y, T_ry = 0.1*r, T_tt = -r.
# Closed-form rotation to Cartesian (default y-axis):
#   T_xx_ref = (x*x - z*z) / sqrt(x*x + z*z)
#   T_zz_ref = -T_xx_ref
#   T_xz_ref = 2*x*z / sqrt(x*x + z*z)
#   T_xy_ref = 0.1*x
#   T_yz_ref = 0.1*z
#   T_yy_ref = y
#
# Test: 6 observed Cartesian components (via tensor-mode Function) vs 6 analytical
# references. Max absolute nodal difference per component goes to a Postprocessor.
# Mesh stays off-axis (xmin = zmin = 1) so the sqrt denominator is bounded away
# from zero.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    xmin = 1
    xmax = 4
    ny = 4
    ymin = 0
    ymax = 4
    nz = 4
    zmin = 1
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
    mesh = 'source_2d_varying_out.e'
    system_variables = 'T_rr T_yy T_ry T_tt'
    timestep = LATEST
    force_replicated_source_mesh = true
  []
[]

[Functions]
  # Tensor-mode rotation (the SUT).
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
  [f_xy]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 0
    component_j = 1
  []
  [f_xz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 0
    component_j = 2
  []
  [f_yz]
    type = Axisymmetric2D3DSolutionFunction
    solution = sol
    from_variables = 'T_rr T_yy T_ry T_tt'
    component_i = 1
    component_j = 2
  []

  # Closed-form analytical references.
  [ref_xx]
    type = ParsedFunction
    expression = '(x*x - z*z) / sqrt(x*x + z*z)'
  []
  [ref_yy]
    type = ParsedFunction
    expression = 'y'
  []
  [ref_zz]
    type = ParsedFunction
    expression = '-(x*x - z*z) / sqrt(x*x + z*z)'
  []
  [ref_xy]
    type = ParsedFunction
    expression = '0.1*x'
  []
  [ref_xz]
    type = ParsedFunction
    expression = '2*x*z / sqrt(x*x + z*z)'
  []
  [ref_yz]
    type = ParsedFunction
    expression = '0.1*z'
  []
[]

[AuxVariables]
  [T_xx_obs]
    family = LAGRANGE
    order = FIRST
  []
  [T_yy_obs]
    family = LAGRANGE
    order = FIRST
  []
  [T_zz_obs]
    family = LAGRANGE
    order = FIRST
  []
  [T_xy_obs]
    family = LAGRANGE
    order = FIRST
  []
  [T_xz_obs]
    family = LAGRANGE
    order = FIRST
  []
  [T_yz_obs]
    family = LAGRANGE
    order = FIRST
  []

  [T_xx_ref]
    family = LAGRANGE
    order = FIRST
  []
  [T_yy_ref]
    family = LAGRANGE
    order = FIRST
  []
  [T_zz_ref]
    family = LAGRANGE
    order = FIRST
  []
  [T_xy_ref]
    family = LAGRANGE
    order = FIRST
  []
  [T_xz_ref]
    family = LAGRANGE
    order = FIRST
  []
  [T_yz_ref]
    family = LAGRANGE
    order = FIRST
  []

  [diff_xx]
    family = LAGRANGE
    order = FIRST
  []
  [diff_yy]
    family = LAGRANGE
    order = FIRST
  []
  [diff_zz]
    family = LAGRANGE
    order = FIRST
  []
  [diff_xy]
    family = LAGRANGE
    order = FIRST
  []
  [diff_xz]
    family = LAGRANGE
    order = FIRST
  []
  [diff_yz]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  # Observed (tensor-mode rotation result).
  [obs_xx]
    type = FunctionAux
    variable = T_xx_obs
    function = f_xx
    execute_on = INITIAL
  []
  [obs_yy]
    type = FunctionAux
    variable = T_yy_obs
    function = f_yy
    execute_on = INITIAL
  []
  [obs_zz]
    type = FunctionAux
    variable = T_zz_obs
    function = f_zz
    execute_on = INITIAL
  []
  [obs_xy]
    type = FunctionAux
    variable = T_xy_obs
    function = f_xy
    execute_on = INITIAL
  []
  [obs_xz]
    type = FunctionAux
    variable = T_xz_obs
    function = f_xz
    execute_on = INITIAL
  []
  [obs_yz]
    type = FunctionAux
    variable = T_yz_obs
    function = f_yz
    execute_on = INITIAL
  []

  # Analytical reference (same FE projection so diff cancels FE error).
  [r_xx]
    type = FunctionAux
    variable = T_xx_ref
    function = ref_xx
    execute_on = INITIAL
  []
  [r_yy]
    type = FunctionAux
    variable = T_yy_ref
    function = ref_yy
    execute_on = INITIAL
  []
  [r_zz]
    type = FunctionAux
    variable = T_zz_ref
    function = ref_zz
    execute_on = INITIAL
  []
  [r_xy]
    type = FunctionAux
    variable = T_xy_ref
    function = ref_xy
    execute_on = INITIAL
  []
  [r_xz]
    type = FunctionAux
    variable = T_xz_ref
    function = ref_xz
    execute_on = INITIAL
  []
  [r_yz]
    type = FunctionAux
    variable = T_yz_ref
    function = ref_yz
    execute_on = INITIAL
  []

  # Absolute differences (correct rotation -> ~0 at every node).
  [d_xx]
    type = ParsedAux
    variable = diff_xx
    coupled_variables = 'T_xx_obs T_xx_ref'
    expression = 'abs(T_xx_obs - T_xx_ref)'
    execute_on = INITIAL
  []
  [d_yy]
    type = ParsedAux
    variable = diff_yy
    coupled_variables = 'T_yy_obs T_yy_ref'
    expression = 'abs(T_yy_obs - T_yy_ref)'
    execute_on = INITIAL
  []
  [d_zz]
    type = ParsedAux
    variable = diff_zz
    coupled_variables = 'T_zz_obs T_zz_ref'
    expression = 'abs(T_zz_obs - T_zz_ref)'
    execute_on = INITIAL
  []
  [d_xy]
    type = ParsedAux
    variable = diff_xy
    coupled_variables = 'T_xy_obs T_xy_ref'
    expression = 'abs(T_xy_obs - T_xy_ref)'
    execute_on = INITIAL
  []
  [d_xz]
    type = ParsedAux
    variable = diff_xz
    coupled_variables = 'T_xz_obs T_xz_ref'
    expression = 'abs(T_xz_obs - T_xz_ref)'
    execute_on = INITIAL
  []
  [d_yz]
    type = ParsedAux
    variable = diff_yz
    coupled_variables = 'T_yz_obs T_yz_ref'
    expression = 'abs(T_yz_obs - T_yz_ref)'
    execute_on = INITIAL
  []
[]

[Postprocessors]
  [max_diff_xx]
    type = NodalExtremeValue
    variable = diff_xx
    value_type = max
    execute_on = INITIAL
  []
  [max_diff_yy]
    type = NodalExtremeValue
    variable = diff_yy
    value_type = max
    execute_on = INITIAL
  []
  [max_diff_zz]
    type = NodalExtremeValue
    variable = diff_zz
    value_type = max
    execute_on = INITIAL
  []
  [max_diff_xy]
    type = NodalExtremeValue
    variable = diff_xy
    value_type = max
    execute_on = INITIAL
  []
  [max_diff_xz]
    type = NodalExtremeValue
    variable = diff_xz
    value_type = max
    execute_on = INITIAL
  []
  [max_diff_yz]
    type = NodalExtremeValue
    variable = diff_yz
    value_type = max
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
