# natural convection through a pipe
# Reference solution in "reference_pipe_natural_convection.py"
# Reference mdot: 0.0792 kg/s
# this input
# iy   mdot
# 10   8.302364e-02
# 20   8.111192e-02
# 40   8.007924e-02
# 80   7.954403e-02
# 160  7.927201e-02
# Convergence to the analytical result is observed

height = 10.0
gravity = 9.81
p0 = 1e5
molar_mass = 29.0e-3
T0 = 328
Ru = 8.3145
Ri = '${fparse Ru / molar_mass}'
density = '${fparse p0 / (Ri * T0)}'
head = '${fparse height * density * gravity}'
k = 25.68e-3
gamma = 1.4

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.1'
    ix = '2'
    dy = '${height}'
    iy = '5'
  []
[]

[GlobalParams]
  rhie_chow_user_object = pins_rhie_chow_interpolator
[]

[FluidProperties]
  [air]
    type = IdealGasFluidProperties
    molar_mass = ${molar_mass}
    k = ${k}
    gamma = ${gamma}
  []
[]

[Modules]
  [NavierStokesFV]
    compressibility = 'weakly-compressible'
    add_energy_equation = true
    gravity = '0 -${gravity} 0'
    density = rho
    dynamic_viscosity = mu
    specific_heat = cp
    thermal_conductivity = k
    initial_velocity = '0 1e-6 0'
    initial_pressure = ${p0}
    initial_temperature = ${T0}
    inlet_boundaries = 'bottom'
    momentum_inlet_types = 'fixed-pressure'
    momentum_inlet_function = '${fparse p0 + head}'
    energy_inlet_types = 'fixed-temperature'
    energy_inlet_function = '${T0}'
    energy_scaling = 1e-5
    wall_boundaries = 'left right'
    momentum_wall_types = 'slip slip'
    energy_wall_types = 'heatflux heatflux'
    energy_wall_function = '300 300'
    outlet_boundaries = 'top'
    momentum_outlet_types = 'fixed-pressure'
    pressure_function = '${fparse p0}'
    momentum_advection_interpolation = 'upwind'
    mass_advection_interpolation = 'upwind'
    friction_types = 'Darcy'
    friction_coeffs = 'Darcy_coef'
    porous_medium_treatment = true
    porosity = porosity
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'lu        NONZERO'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  end_time = 1e4
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1
    growth_factor = 2
    iteration_window = 2
    optimal_iterations = 6
  []
[]

[Functions]
  [mu_rampdown_fn]
    type = PiecewiseLinear
    x = '0    0.5  1   5  10 100 1000 2000'
    y = '1000 1000 100 10 1  1   1    0'
  []
[]

[Materials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = air
    pressure = pressure
    T_fluid = T_fluid
    speed = speed
    force_define_density = true
    mu_rampdown = 'mu_rampdown_fn'
    characteristic_length = 1
    porosity = porosity
  []

  [scalar_props]
    type = ADGenericFunctorMaterial
    prop_names = 'porosity'
    prop_values = '1      '
  []

  [vector_props]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'Darcy_coef'
    prop_values = '1.3 1.3 1.3'
  []
[]

[AuxVariables]
  [rho_var]
    type = MooseVariableFVReal
  []

  [cp_var]
    type = MooseVariableFVReal
  []

  [rho_cp_T_fluid_var]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [rho_var_aux]
    type = ADFunctorElementalAux
    variable = rho_var
    functor = rho
  []

  [cp_var_aux]
    type = ADFunctorElementalAux
    variable = cp_var
    functor = cp
  []

  [rho_cp_T_fluid_var_aux]
    type = ParsedAux
    variable = rho_cp_T_fluid_var
    coupled_variables = 'rho_var cp_var T_fluid'
    expression = 'rho_var * cp_var * T_fluid'
  []
[]

[Postprocessors]
  [inlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = rho
    boundary = bottom
    advected_interp_method = average
  []

  [outlet_mfr]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = rho
    boundary = top
    advected_interp_method = average
  []

  [inlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = rho_cp_T_fluid_var
    boundary = bottom
    advected_interp_method = average
  []

  [outlet_energy]
    type = VolumetricFlowRate
    vel_x = superficial_vel_x
    vel_y = superficial_vel_y
    advected_quantity = rho_cp_T_fluid_var
    boundary = top
    advected_interp_method = average
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]
