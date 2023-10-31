# Operating conditions
u_inlet = 1
p_outlet = 1e5
T_initial = 300
1d_outlet_id = 49

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  # [T_fluid]
  #   type = MooseVariableFVReal
  #   initial_condition = ${T_initial}
  # []
[]

[Components]
  [fixed_inlet]
    type = InletVelocityTemperature1Phase
    input = 'comp1:in'
    vel = ${u_inlet}
    T = ${T_initial}
  []

  [comp1]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    gravity_vector = '0 0 0'
    length = 1.0
    n_elems = 50
    A = 1.0

    initial_T = ${T_initial}
    initial_p = ${p_outlet}
    initial_vel = ${u_inlet}

    f = 10.0
    closures = simple_closures
    fp = fp
  []

  [1d_outlet]
    type = Outlet1PhaseFromWCNSFV
    input = 'comp1:out'
    p = 'pressure_md_left'
  []

  [comp2]
    type = FileMeshWCNSFVComponent
    file = rectangle.e
    position = '0 0 0'
    verbose = true

    add_flow_equations = true
    add_energy_equation = true
    add_scalar_equations = false
    turbulence_model = 'mixing-length'

    # Avoid naming conflicts between NS and THM
    velocity_variable = 'vx vy'
    density = 'rho_ns'

    inlet_boundaries = 'comp2:left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_functors = 'velx_1d 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_functors = 'T_1d_elem'

    wall_boundaries = 'comp2:top comp2:bottom'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_functors = '0 0'

    outlet_boundaries = 'comp2:right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_functors = ${p_outlet}

    initial_velocity = '${u_inlet} 0 0'
    initial_pressure = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'

    # The junction adds more boundary conditions
    # boundary_conditions_all_set = false
  []
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'mu k cp'
    prop_values = '1 1 1'
    block = 'comp2:body'
  []
  # Density for NS
  [rho_ns]
    type = RhoFromPTFunctorMaterial
    fp = fp
    density_name = rho_ns
    pressure = 'pressure'
    temperature = 'T_fluid'
  []

  # Inlet of comp2, get v from 1d rhouA and rhoA
  [1d_v]
    type = ADParsedFunctorMaterial
    property_name = 'vel_1d'
    expression = 'rhouA / rhoA'
    functor_names = 'rhouA rhoA'
  []
  [1d_outlet_vx]
    type = ADFixedElemValueFunctorMaterial
    functor_in = 'vel_1d'
    functor_name = 'velx_1d'
    elem_id = ${1d_outlet_id}
  []

  # Inlet of comp2, get T from fluid properties
  [define_pressure]
    type = GeneralFunctorFluidPropsVE
    fp = fp
    pressure_name = 'pressure_ad_elem'
    temperature_name = 'T_1d'
    rhoA = 'rhoA'
    rhoEA = 'rhoEA'
    rhouA = 'rhouA'
    A = 'A'
    alpha = 1
    block = 'comp1'
  []
  [1d_outlet_T]
    type = ADFixedElemValueFunctorMaterial
    functor_in = 'T_1d'
    functor_name = 'T_1d_elem'
    elem_id = ${1d_outlet_id}
  []

  # Inlet of comp2: get pressure from multiD sim
  [pressure_left]
    type = ADBoundaryIntegralFunctorMaterial
    functor_in = 'pressure'
    functor_name = 'pressure_md_left'
    integration_boundary = 'comp2:left'
    compute_average = true
  []
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.4
    # emit_on_nan = none
  []
  # [fp]
  #   # Nearly incompressible
  #   type = SimpleFluidProperties
  #   cp = 4194
  #   density0 = 1000
  #   bulk_modulus = 2e+09
  # []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       NONZERO               mumps'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-5
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  verbose = true
[]

[Debug]
  show_var_residual_norms = true
[]

[Problem]
  verbose_setup = true
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [mid-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp2:left'
  []
  [outlet]
    type = SideAverageValue
    variable = pressure
    boundary = 'comp2:right'
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
