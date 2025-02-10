mu = 2.6
rho = 1.0
advected_interp_method = 'upwind'

# A large diffusion term makes the balance worse
# because some scalar diffuses to the inlet/outlet and
# the postprocessors are only measuring the advective term
k1 = 0
k2 = 0

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 1
    dx = '1 1 1'
    ix = '10 10 10'
    subdomain_id = '0 1 0'
  []
[]

[Problem]
  linear_sys_names = 'u_system pressure_system s1_system s2_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        velocity_variable = 'vel_x'
        pressure_variable = 'pressure'

        initial_velocity = '0.5 0 0'
        initial_pressure = '0.2'

        density = ${rho}
        dynamic_viscosity = ${mu}

        # use inlet for moving wall to match the reference input
        # we could also use a noslip BC with a velocity wall functor
        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '1'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '1.4'

        orthogonality_correction = false
        pressure_two_term_bc_expansion = false
        momentum_two_term_bc_expansion = false
        momentum_advection_interpolation = ${advected_interp_method}
      []
    []
    [ScalarTransportSegregated]
      [scalar]
        passive_scalar_names = 'scalar1 scalar2'
        system_names = 's1_system s2_system'
        initial_scalar_variables = '1.1 3'

        passive_scalar_diffusivity = '${k1} ${k2}'

        passive_scalar_coupled_source = 'q1; q2'

        passive_scalar_inlet_types = 'fixed-value fixed-value'
        passive_scalar_inlet_functors = '1; 2'

        passive_scalar_advection_interpolation = ${advected_interp_method}
        passive_scalar_two_term_bc_expansion = false
        use_nonorthogonal_correction = false
      []
    []
  []
[]

# Note, we do not block restrict these source variables because
# there is not passive_scalar_coupled_source_blocks parameter at this time
[AuxVariables]
  [q1]
    type = MooseLinearVariableFVReal
  []
  [q2]
    type = MooseLinearVariableFVReal
  []
[]

[FVICs]
  [q1]
    type = FVConstantIC
    variable = q1
    value = 1
    block = 1
  []
  [q2]
    type = FVConstantIC
    variable = q2
    value = 2
    block = 1
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  passive_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  passive_scalar_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system'
  pressure_system = 'pressure_system'
  passive_scalar_systems = 's1_system s2_system'
  momentum_equation_relaxation = 0.4
  pressure_variable_relaxation = 0.3
  passive_scalar_equation_relaxation = '0.9 0.9'
  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  passive_scalar_absolute_tolerance = '1e-9 1e-9'

  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  passive_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  passive_scalar_petsc_options_value = 'hypre boomeramg'

  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  # hide = 'pressure vel_x'
  [csv]
    type = CSV
    hide = 'balance_s1 balance_s2'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  subtract_mesh_velocity = false
  advected_interp_method = ${advected_interp_method}
[]

[Postprocessors]
  [s1_in]
    type = VolumetricFlowRate
    boundary = left
    vel_x = vel_x
    advected_quantity = 'scalar1'
  []
  [s2_in]
    type = VolumetricFlowRate
    boundary = left
    vel_x = vel_x
    advected_quantity = 'scalar2'
  []
  [s1_out]
    type = VolumetricFlowRate
    boundary = right
    vel_x = vel_x
    advected_quantity = 'scalar1'
  []
  [s2_out]
    type = VolumetricFlowRate
    boundary = right
    vel_x = vel_x
    advected_quantity = 'scalar2'
  []
  [balance_s1]
    type = ParsedPostprocessor
    expression = 's1_out + s1_in - 1'
    pp_names = 's1_in s1_out'
  []
  [balance_s2]
    type = ParsedPostprocessor
    expression = 's2_out + s2_in - 2'
    pp_names = 's2_in s2_out'
  []
[]
