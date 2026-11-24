# Physics syntax version of porous-baffle-1d.i. The flow Physics creates the porous Rhie-Chow
# object, the Bernoulli pressure-jump model on the baffles, the jump-aware gradient methods and the
# porous momentum kernels, so this input reproduces the results of the explicit object setup.

mu = 0.0
rho = 2.0
forchheimer_value = 50
inlet_u = 0.1

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5 0.5'
    ix = '200 200 200'
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
  linear_sys_names = 'u_system pressure_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        velocity_variable = 'superficial_u'
        pressure_variable = 'pressure'

        density = ${rho}
        dynamic_viscosity = ${mu}

        porous_medium_treatment = true
        porosity = porosity
        porosity_interface_pressure_treatment = bernoulli
        pressure_drop_sidesets = 'baffle baffle2'
        pressure_jump_relaxation = 0.1
        pressure_gradient_relaxation = 1.0

        friction_blocks = '2'
        friction_types = 'Forchheimer'
        friction_coeffs = 'forch'

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
  []
[]

[FunctorMaterials]
  [forch]
    type = GenericVectorFunctorMaterial
    prop_names = forch
    prop_values = '${forchheimer_value} ${forchheimer_value} ${forchheimer_value}'
  []
  [porosity]
    type = PiecewiseByBlockFunctorMaterial
    prop_name = porosity
    subdomain_to_prop_value = '1 0.5 2 1.0 3 0.5'
  []
[]

[Postprocessors]
  [p_left]
    type = SideAverageValue
    variable = pressure
    boundary = left
  []
  [p_right]
    type = SideAverageValue
    variable = pressure
    boundary = right
  []
  [p_jump]
    type = ParsedPostprocessor
    expression = 'p_left - p_right'
    pp_names = 'p_left p_right'
  []
  # These points are the centers of the cells adjacent to x = 0.5 and x = 1.0. The mesh spacing
  # is 0.5 / 200 = 0.0025, so the adjacent centers are offset by half that distance.
  [p_baffle_1_left]
    type = PointValue
    variable = pressure
    point = '0.49875 0 0'
  []
  [p_baffle_1_right]
    type = PointValue
    variable = pressure
    point = '0.50125 0 0'
  []
  [p_baffle_2_left]
    type = PointValue
    variable = pressure
    point = '0.99875 0 0'
  []
  [p_baffle_2_right]
    type = PointValue
    variable = pressure
    point = '1.00125 0 0'
  []
  [p_baffle_1_jump]
    type = ParsedPostprocessor
    expression = 'p_baffle_1_right - p_baffle_1_left'
    pp_names = 'p_baffle_1_right p_baffle_1_left'
  []
  [p_baffle_2_jump]
    type = ParsedPostprocessor
    expression = 'p_baffle_2_right - p_baffle_2_left'
    pp_names = 'p_baffle_2_right p_baffle_2_left'
  []
  [u_block_1]
    type = ElementAverageValue
    variable = superficial_u
    block = 1
  []
  [u_block_2]
    type = ElementAverageValue
    variable = superficial_u
    block = 2
  []
  [u_block_jump]
    type = ParsedPostprocessor
    expression = 'u_block_1 - u_block_2'
    pp_names = 'u_block_1 u_block_2'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = pins_rhie_chow_interpolator
  momentum_systems = 'u_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.3
  pressure_variable_relaxation = 0.1
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-8
  # See porous-baffle-1d.i: the momentum residual levels off at round-off for uniform velocity
  momentum_absolute_tolerance = 1
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
