mu = 1.1
rho = 1.1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = -1
    ymax = 1
    nx = 50
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'incompressible'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '1 1 0'
        initial_pressure = 0.0

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '1 0'
        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'
        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '0'

        friction_types = 'darcy'
        friction_coeffs = 'friction_coefficient'

        momentum_advection_interpolation = 'average'
      []
    []
  []
[]

[FunctorMaterials]
  [const]
    type = ADGenericFunctorMaterial
    prop_names = 'rho mu'
    prop_values = '${rho} ${mu}'
  []
  [friction_coefficient]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'friction_coefficient'
    prop_values = '25 25 25'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  # Systems
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3

  # We need to converge the problem to show conservation
  num_iterations = 200
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  # print_fields = true
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  csv = true
[]

[Postprocessors]
  [dp]
    type = PressureDrop
    pressure = 'pressure'
    upstream_boundary = 'left'
    downstream_boundary = 'right'
    boundary = 'left right'
  []
[]
