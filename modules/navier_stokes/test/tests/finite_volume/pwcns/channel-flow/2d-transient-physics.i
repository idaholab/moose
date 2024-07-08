# Solid properties
cp_s = 2
rho_s = 4
k_s = '${fparse 1e-2 / 0.5}'
h_fs = 10

# thermal diffusivity is divided by 0.5 to match the reference using the action

# Operating conditions
u_inlet = 1
T_inlet = 200
p_outlet = 10
top_side_temperature = 150

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 1
    nx = 20
    ny = 5
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
  [velocity_norm]
    type = MooseVariableFVReal
  []
[]

[FluidProperties]
  [fp]
    type = FlibeFluidProperties
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'weakly-compressible'
        porous_medium_treatment = true
        define_variables = true

        pressure_variable = 'pressure'

        density = 'rho'
        dynamic_viscosity = 'mu'

        initial_velocity = '${u_inlet} 1e-6 0'
        initial_pressure = '${p_outlet}'

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_function = '${u_inlet} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip symmetry'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_function = '${p_outlet}'

        mass_advection_interpolation = 'average'
        momentum_advection_interpolation = 'average'
      []
    []
    [FluidHeatTransfer]
      [fluid]
        thermal_conductivity = 'k'
        effective_conductivity = true
        specific_heat = 'cp'

        initial_temperature = '${T_inlet}'

        # See 'flow' for inlet boundaries
        energy_inlet_types = 'fixed-temperature'
        energy_inlet_function = '${T_inlet}'

        # See 'flow' for wall boundaries
        energy_wall_types = 'heatflux heatflux'
        energy_wall_function = '0 0'

        ambient_convection_alpha = 'h_cv'
        ambient_temperature = 'T_solid'

        energy_advection_interpolation = 'average'
      []
    []

    [SolidHeatTransfer]
      [solid]
        block = 0
        initial_temperature = 100
        transient = true
        # To match the previous test results
        solid_temperature_two_term_bc_expansion = true

        thermal_conductivity_solid = '${k_s}'
        cp_solid = ${cp_s}
        rho_solid = ${rho_s}

        fixed_temperature_boundaries = 'top'
        boundary_temperatures = '${top_side_temperature}'

        ambient_convection_alpha = 'h_cv'
        ambient_convection_temperature = 'T_fluid'
        verbose = true
      []
    []
  []
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'h_cv'
    prop_values = '${h_fs}'
  []
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = 'pressure'
    T_fluid = 'T_fluid'
    speed = 'velocity_norm'

    # To initialize with a high viscosity
    mu_rampdown = 'mu_rampdown'

    # For porous flow
    characteristic_length = 1
    porosity = 'porosity'
  []
[]

[Functions]
  [mu_rampdown]
    type = PiecewiseLinear
    x = '1 2 3 4'
    y = '1e3 1e2 1e1 1'
  []
[]

[AuxKernels]
  [speed]
    type = ParsedAux
    variable = 'velocity_norm'
    coupled_variables = 'superficial_vel_x superficial_vel_y porosity'
    expression = 'sqrt(superficial_vel_x*superficial_vel_x + superficial_vel_y*superficial_vel_y) / porosity'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm      100                lu           NONZERO'
  line_search = 'none'
  nl_rel_tol = 1e-12

  end_time = 3.0
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [inlet-p]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [outlet-u]
    type = SideAverageValue
    variable = superficial_vel_x
    boundary = 'right'
  []
  [outlet-temp]
    type = SideAverageValue
    variable = T_fluid
    boundary = 'right'
  []
  [solid-temp]
    type = ElementAverageValue
    variable = T_solid
  []
[]

[Outputs]
  exodus = true
  csv = false
[]
