rho_initial=1.29
p_initial=1.01e5
T=273.15
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# prescribe inlet rho = initial rho
rho_in=${rho_initial}
# u refers to the superficial velocity
u_in=10
mass_flux_in=${fparse u_in * rho_in}
et_initial=${fparse e_initial + u_in * u_in / 2}
rho_et_initial=${fparse rho_initial * et_initial}

[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 100
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
    initial_condition = ${mass_flux_in}
    # initial_condition = 1e-15
    scaling = ${fparse 1. / mass_flux_in}
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = ${rho_et_initial}
    scaling = ${fparse 1. / rho_et_initial}
  []
[]

[AuxVariables]
  [vel_x]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
  [normalized_vel_x]
    type = MooseVariableFVReal
  []
  [normalized_rho_u]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [vel_x]
    type = ADMaterialRealAux
    variable = vel_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = pressure
    execute_on = 'timestep_end'
  []
  [temperature]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
    execute_on = 'timestep_end'
  []
  [normalized_vel_x]
    type = ParsedAux
    variable = normalized_vel_x
    function = 'vel_x / ${u_in}'
    args = 'vel_x'
    execute_on = 'timestep_end'
  []
  [normalized_rho_u]
    type = ParsedAux
    variable = normalized_rho_u
    function = 'rho_u / ${u_in}'
    args = 'rho_u'
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = NSFVMassFluxAdvection
    variable = rho
    advected_quantity = 1
  []

  [momentum_advection]
    type = NSFVMassFluxAdvection
    variable = rho_u
    advected_quantity = 'vel_x'
  []
  [momentum_pressure]
    type = PCNSFVMomentumPressureFlux
    variable = rho_u
    momentum_component = 'x'
  []
  [friction]
    type = FVReaction
    variable = rho_u
    rate = 100
  []

  [energy_advection]
    type = NSFVMassFluxAdvection
    variable = rho_et
    advected_quantity = 'ht'
  []
  [heat]
    type = FVBodyForce
    variable = rho_et
    value = 1e6
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
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho
    advected_quantity = 1
  []
  [rho_u_left]
    type = FVDirichletBC
    boundary = 'left'
    variable = rho_u
    value = ${mass_flux_in}
  []
  [rho_u_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho_u
    advected_quantity = 'vel_x'
  []
  [rho_u_pressure_right]
    type = PNSFVMomentumSpecifiedPressureBC
    boundary = 'right'
    variable = rho_u
    momentum_component = 'x'
    pressure = ${p_initial}
  []
  [rho_et_left]
    type = PNSFVFluidEnergySpecifiedTemperatureBC
    variable = rho_et
    boundary = 'left'
    superficial_rhou = ${mass_flux_in}
    temperature = ${T}
  []
  [rho_et_right]
    type = NSFVMassFluxAdvectionBC
    boundary = 'right'
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_u
    fp = fp
    porosity = porosity
  []
  [zero]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
  petsc_options_value = 'lu       mumps                      NONZERO'
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_var_residual_norms = true
[]
