# Adds a passive scalar on the flow block to reconstructed-gradient.i. The FaceInfo element on
# the internal inlet is the inactive cell, so the advective inflow there is assembled from the
# FaceInfo neighbor. The uniform inflow of 1 must be advected unchanged through the channel.

[Problem]
  linear_sys_names = 'u_system v_system pressure_system s_system'
[]

[Variables]
  [scalar]
    type = MooseLinearVariableFVReal
    solver_sys = s_system
    initial_condition = 0
    block = ${restricted_blocks}
  []
[]

[FVInterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
[]

[LinearFVKernels]
  [scalar_advection]
    type = LinearFVScalarAdvection
    variable = scalar
    advected_interp_method_name = upwind
    rhie_chow_user_object = rc
  []
[]

[LinearFVBCs]
  [inlet-scalar]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = internal_inlet
    variable = scalar
    functor = 1
  []
  [outlet-scalar]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = internal_outlet
    variable = scalar
    use_two_term_expansion = false
  []
[]

[Functions]
  [exact-one]
    type = ParsedFunction
    expression = 1
  []
[]

[Postprocessors]
  [scalar-error]
    type = ElementL2FunctorError
    approximate = scalar
    exact = exact-one
    block = ${flow_block}
  []
[]

[Executioner]
  passive_scalar_systems = s_system
  passive_scalar_equation_relaxation = 1.0
  passive_scalar_l_abs_tol = 1e-12
  passive_scalar_l_tol = 0
  passive_scalar_absolute_tolerance = 1e-10
  passive_scalar_petsc_options_iname = '-pc_type'
  passive_scalar_petsc_options_value = 'lu'
[]
