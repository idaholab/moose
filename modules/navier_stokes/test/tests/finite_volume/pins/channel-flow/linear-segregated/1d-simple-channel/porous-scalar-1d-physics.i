# Passive scalar advected through a porosity jump bounded by two pressure baffles. The scalar is a
# concentration per unit fluid volume, so the steady balance without diffusion is
#   d(U c)/dx = eps S,
# where the superficial velocity U is uniform. With the inlet value c_in and a uniform source S,
#   c(x) = c_in + (S / U) int_0^x eps dx'.
# The porosity is 0.5, 1 and 0.5 on three 0.5-m blocks, so int eps dx is 0.25 at the first baffle,
# 0.75 at the second and 1 at the outlet. With U = 0.1, c_in = 1 and S = 0.2 this gives
# c = 1.5 at the first baffle, 2.5 at the second and c_out = 3. Upwinding is exact for this
# piecewise-linear profile at the cell faces, so the outlet value is exact. The conservation
# check compares the outflow U (c_out - c_in) to the integrated source int eps S dx = 0.2.
# The upwind cell value is the value at the left face of the cell, so across a baffle the values
# of the two adjacent cells differ only by the source of the upwind cell, dx eps S / U: 0.01 for a
# cell with eps = 0.5 and 0.02 for eps = 1, with dx = 0.01. The scalar does not jump at the baffles.

rho = 2.0
inlet_u = 0.1
c_in = 1
source = 0.2

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5 0.5'
    ix = '50 50 50'
    subdomain_id = '1 2 3'
  []
  [baffle]
    type = SideSetsBetweenSubdomainsGenerator
    input = mesh
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'baffle'
  []
  [baffle2]
    type = SideSetsBetweenSubdomainsGenerator
    input = baffle
    primary_block = '2'
    paired_block = '3'
    new_boundary = 'baffle2'
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system scalar_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        velocity_variable = 'superficial_u'
        pressure_variable = 'pressure'

        density = ${rho}
        dynamic_viscosity = 0

        porous_medium_treatment = true
        porosity = porosity
        porosity_interface_pressure_treatment = bernoulli
        pressure_drop_sidesets = 'baffle baffle2'
        pressure_jump_relaxation = 0.1
        pressure_gradient_relaxation = 1.0

        initial_velocity = '${inlet_u} 0 0'
        initial_pressure = 0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${inlet_u}'
        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        momentum_advection_interpolation = 'average'
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        orthogonality_correction = false
      []
    []
    [ScalarTransportSegregated]
      [scalar]
        passive_scalar_names = 'c'
        system_names = 'scalar_system'
        initial_scalar_variables = '${c_in}'

        passive_scalar_source = 'scalar_source'

        passive_scalar_inlet_types = 'fixed-value'
        passive_scalar_inlet_functors = '${c_in}'

        passive_scalar_advection_interpolation = 'upwind'
        passive_scalar_two_term_bc_expansion = false
        use_nonorthogonal_correction = false
      []
    []
  []
[]

[FunctorMaterials]
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.5 2 1.0 3 0.5'
  []
  [source]
    type = GenericFunctorMaterial
    prop_names = 'scalar_source'
    prop_values = '${source}'
  []
  # Source density per unit total volume, for the conservation check
  [porous_source_density]
    type = ParsedFunctorMaterial
    property_name = porous_source_density
    functor_names = 'porosity scalar_source'
    expression = 'porosity * scalar_source'
  []
[]

[Postprocessors]
  [c_out]
    type = SideAverageValue
    variable = c
    boundary = right
  []
  # Cell centers on each side of the baffles (mesh spacing 0.01)
  [c_baffle_1_left]
    type = PointValue
    variable = c
    point = '0.495 0 0'
  []
  [c_baffle_1_right]
    type = PointValue
    variable = c
    point = '0.505 0 0'
  []
  [c_baffle_2_left]
    type = PointValue
    variable = c
    point = '0.995 0 0'
  []
  [c_baffle_2_right]
    type = PointValue
    variable = c
    point = '1.005 0 0'
  []
  [outflow]
    type = ParsedPostprocessor
    expression = '${inlet_u} * (c_out - ${c_in})'
    pp_names = 'c_out'
  []
  [porous_source]
    type = ElementIntegralFunctorPostprocessor
    functor = porous_source_density
  []
  [conservation_error]
    type = ParsedPostprocessor
    expression = 'outflow - porous_source'
    pp_names = 'outflow porous_source'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  passive_scalar_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  passive_scalar_l_tol = 0
  rhie_chow_user_object = pins_rhie_chow_interpolator
  momentum_systems = 'u_system'
  pressure_system = pressure_system
  passive_scalar_systems = 'scalar_system'
  momentum_equation_relaxation = 0.3
  pressure_variable_relaxation = 0.1
  passive_scalar_equation_relaxation = 1.0
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-8
  # See porous-baffle-1d.i: the momentum residual levels off at round-off for uniform velocity
  momentum_absolute_tolerance = 1
  passive_scalar_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
