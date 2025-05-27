mu = 0.01
rho = 1.1
k = 0.0005
cp = 10
h_conv = 5

inlet_temp = 300
advected_interp_method = 'upwind'

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    ymin = 0
    ymax = 0.1
    xmax = 0.1
  []
  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    input = generated_mesh
    block_name = subdomain1
    bottom_left = '0.04 0.04 0'
    block_id = 1
    top_right = '0.06 0.06 0'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
  [delete]
    type = BlockDeletionGenerator
    input = interface
    block = 1
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'
        block = '0'

        velocity_variable = 'vel_x vel_y'

        density = ${rho}
        dynamic_viscosity = ${mu}

        initial_velocity = '0.1 0.01 0'
        initial_pressure = '0.2'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '0.1 0'

        wall_boundaries = 'top bottom interface'
        momentum_wall_types = 'noslip noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '1.4'

        momentum_advection_interpolation = ${advected_interp_method}
        momentum_two_term_bc_expansion = false
        pressure_two_term_bc_expansion = false
        orthogonality_correction = false
      []
    []
    [FluidHeatTransferSegregated]
      [energy]
        coupled_flow_physics = flow
        block = '0'

        thermal_conductivity = '${k}'
        specific_heat = '${cp}'

        initial_temperature = '${inlet_temp}'

        energy_inlet_types = 'fixed-temperature'
        energy_inlet_functors = '${inlet_temp}'
        energy_wall_types = 'heatflux heatflux convection'
        energy_wall_functors = '0 0 boundary_value'

        energy_advection_interpolation = ${advected_interp_method}
        energy_two_term_bc_expansion = false
      []
    []
  []
[]

[FunctorMaterials]
  [rhocpT]
    property_name = 'rhocpT'
    type = ParsedFunctorMaterial
    functor_names = 'T_fluid'
    expression = '${rho}*${cp}*T_fluid'
  []
  [h]
    type = GenericFunctorMaterial
    prop_names = 'htc'
    prop_values = '${h_conv}'
  []
[]

[Functions]
  [boundary_value]
    type = ConstantFunction
    value = 350
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 1.0
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
