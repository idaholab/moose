# Fluid properties
mu = 1
rho = 1
cp = 1
k = 1e-3

# Operating conditions
u_inlet = 1
T_inlet = 200
p_outlet = 10

# expected temperature at outlet
#
# mdot * cp dT = Qdot
# Qdot = 500
# mdot = 1
# cp = 1
# dT = 500

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5  0.1 4.9'
    ix = '10 3 10'
    dy = '0.5 0.5'
    iy = '2 2'
    subdomain_id = '1 2 3 1 3 3'
  []

  [interior]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '2 3'
    new_boundary = 'interior'
    input = gen
  []
[]

[AuxVariables]
  [porosity]
    type = MooseVariableFVReal
  []

  [hsrc]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [porosity_1]
    type = ConstantIC
    variable = porosity
    value = 0.5
    block = '1 3'
  []

  [porosity_2]
    type = ConstantIC
    variable = porosity
    value = 0.1
    block = 2
  []

  [hsrc]
    type = ConstantIC
    variable = hsrc
    value = 100
    block = 1
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'incompressible'
    porous_medium_treatment = true
    add_energy_equation = true

    density = 'rho'
    dynamic_viscosity = 'mu'
    thermal_conductivity = 'k'
    specific_heat = 'cp'
    porosity = 'porosity'

    # Reference file sets effective_conductivity by default that way
    # so the conductivity is multiplied by the porosity in the kernel
    effective_conductivity = false

    initial_velocity = '${u_inlet} 1e-6 0'
    initial_pressure = ${p_outlet}
    initial_temperature = ${T_inlet}

    inlet_boundaries = 'left'
    momentum_inlet_types = 'fixed-velocity'
    momentum_inlet_function = '${u_inlet} 0'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '${T_inlet}'

    wall_boundaries = 'top bottom'
    momentum_wall_types = 'noslip symmetry'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '0 0'

    outlet_boundaries = 'right'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${p_outlet}'

    mass_advection_interpolation = 'upwind'
    momentum_advection_interpolation = 'upwind'
    energy_advection_interpolation = 'upwind'

    external_heat_source = hsrc
  []
[]

[FunctorMaterials]
  [constants]
    type = ADGenericFunctorMaterial
    prop_names = 'cp rho mu k'
    prop_values = '${cp} ${rho} ${mu} ${k}'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
  end_time = 1000
  dt = 10
  num_steps = 5
[]

# Some basic Postprocessors to examine the solution
[Postprocessors]
  [Qdot]
    type = ElementIntegralVariablePostprocessor
    variable = hsrc
  []

  [mass-flux-weighted-T-out]
    type = MassFluxWeightedFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = T_fluid
    density = rho
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    boundary = 'right'
  []

  [mass-flux-weighted-T-interior]
    type = MassFluxWeightedFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = T_fluid
    density = rho
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    boundary = 'interior'
  []

  [mdot]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = rho
    rhie_chow_user_object = 'pins_rhie_chow_interpolator'
    boundary = 'right'
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
[]

[Outputs]
  csv = true
[]
