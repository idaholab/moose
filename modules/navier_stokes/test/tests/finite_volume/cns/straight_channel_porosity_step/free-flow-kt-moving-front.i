rho_initial=1.29
p_initial=1.01e5
T=273.15
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# No bulk velocity in the domain initially
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
# prescribe inlet rho = initial rho
rho_in=${rho_initial}
# u refers to the superficial velocity
u_in=1
mass_flux_in=${fparse u_in * rho_in}

[GlobalParams]
  vel = velocity
  interp_method = 'upwind'
  fp = fp
  advected_interp_method = 'upwind'
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 100
  []
  [pt5]
    input = cartesian
    type = SubdomainBoundingBoxGenerator
    bottom_left = '2 0 0'
    top_right = '4 1 0'
    block_id = 1
  []
  [pt25]
    input = pt5
    type = SubdomainBoundingBoxGenerator
    bottom_left = '4 0 0'
    top_right = '6 1 0'
    block_id = 2
  []
  [pt5_again]
    input = pt25
    type = SubdomainBoundingBoxGenerator
    bottom_left = '6 0 0'
    top_right = '8 1 0'
    block_id = 3
  []
  [one_again]
    input = pt5_again
    type = SubdomainBoundingBoxGenerator
    bottom_left = '8 0 0'
    top_right = '10 1 0'
    block_id = 4
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [rho]
    type = MooseVariableFVReal
    initial_condition = ${rho_initial}
  []
  [rho_u]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = ${rho_et_initial}
    scaling = 1e-5
  []
[]

[AuxVariables]
  [specific_volume]
    type = MooseVariableFVReal
  []
  [vel_x]
    type = MooseVariableFVReal
  []
  [specific_internal_energy]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [mach]
    type = MooseVariableFVReal
  []
  [mass_flux]
    type = MooseVariableFVReal
  []
  [momentum_flux]
    type = MooseVariableFVReal
  []
  [enthalpy_flux]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
  [courant]
    type = MooseVariableFVReal
  []
  [worst_courant]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [specific_volume]
    type = SpecificVolumeAux
    variable = specific_volume
    rho = rho
    execute_on = 'timestep_end'
  []
  [vel_x]
    type = NSVelocityAux
    variable = vel_x
    rho = rho
    momentum = rho_u
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (vel_x * vel_x) / 2'
    args = 'rho_et rho vel_x'
    execute_on = 'timestep_end'
  []
  [pressure]
    type = PressureAux
    variable = pressure
    v = specific_volume
    e = specific_internal_energy
    execute_on = 'timestep_end'
  []
  [mach]
    type = NSMachAux
    variable = mach
    vel_x = vel_x
    e = specific_internal_energy
    specific_volume = specific_volume
    execute_on = 'timestep_end'
    fluid_properties = 'fp'
  []
  [mass_flux]
    type = ParsedAux
    variable = mass_flux
    function = 'rho_u'
    args = 'rho_u'
    execute_on = 'timestep_end'
  []
  [momentum_flux]
    type = ParsedAux
    variable = momentum_flux
    function = 'vel_x * rho_u + pressure'
    args = 'vel_x rho_u pressure'
    execute_on = 'timestep_end'
  []
  [enthalpy_flux]
    type = ParsedAux
    variable = enthalpy_flux
    function = 'vel_x * (rho_et + pressure)'
    args = 'vel_x rho_et pressure'
    execute_on = 'timestep_end'
  []
  [temperature]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
    execute_on = 'timestep_end'
  []
  [courant]
    type = INSCourant
    variable = courant
    u = vel_x
    execute_on = 'timestep_end'
  []
  [worst_courant]
    type = Courant
    variable = worst_courant
    u = vel_x
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_time]
    type = FVTimeKernel
    variable = rho
  []
  [mass_advection]
    type = CNSFVLaxFriedrichs
    variable = rho
    eqn = "mass"
  []

  [mommentum_time]
    type = FVTimeKernel
    variable = rho_u
  []
  [momentum_advection]
    type = CNSFVLaxFriedrichs
    variable = rho_u
    eqn = "momentum"
    momentum_component = 'x'
  []

  [energy_time]
    type = FVTimeKernel
    variable = rho_et
  []
  [energy_advection]
    type = CNSFVLaxFriedrichs
    variable = rho_et
    eqn = "energy"
  []
[]

[FVBCs]
  [rho_left]
    type = FVNeumannBC
    boundary = 'left'
    variable = rho
    value = ${mass_flux_in}
  []
  [rho_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho
  []
  [rho_u_left]
    type = FVDirichletBC
    boundary = 'left'
    variable = rho_u
    value = ${mass_flux_in}
  []
  [rho_u_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_u
    advected_quantity = 'rhou'
  []
  [rho_u_pressure_right]
    type = CNSFVMomSpecifiedPressureBC
    boundary = 'right'
    variable = rho_u
    momentum_component = 'x'
    specified_pressure = ${p_initial}
  []
  [rho_et_left]
    type = NSFVFluidEnergySpecifiedTemperatureBC
    variable = rho_et
    boundary = 'left'
    rhou = ${mass_flux_in}
    temperature = ${T}
  []
  [rho_et_right]
    type = FVMatAdvectionOutflowBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'rho_ht'
  []
[]

[Materials]
  [var_mat]
    type = ConservedVarValuesMaterial
    rho = rho
    rho_et = rho_et
    rhou = rho_u
    fp = fp
  []
[]

[Executioner]
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_max_its = 10
  nl_abs_tol = 1e-10

  [TimeIntegrator]
    type = ActuallyExplicitEuler
  []
  type = Transient
  dt = 2e-4
  num_steps = 100
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  abort_on_solve_fail = true
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'initial timestep_end'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
