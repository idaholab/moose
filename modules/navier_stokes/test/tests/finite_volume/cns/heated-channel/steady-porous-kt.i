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
    type = PCNSFVLaxFriedrichs
    variable = rho
    eqn = "mass"
  []

  [momentum_advection]
    type = PCNSFVLaxFriedrichs
    variable = rho_u
    eqn = "momentum"
    momentum_component = 'x'
  []

  [energy_advection]
    type = PCNSFVLaxFriedrichs
    variable = rho_et
    eqn = "energy"
  []
  [heat]
    type = FVBodyForce
    variable = rho_et
    value = 1e6
  []
[]

[FVBCs]
  [rho_left]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'left'
    variable = rho
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'mass'
  []
  [rhou_left]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'left'
    variable = rho_u
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_left]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'left'
    variable = rho_et
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'energy'
  []
  [rho_right]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'right'
    variable = rho
    pressure = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'right'
    variable = rho_u
    pressure = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_right]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'right'
    variable = rho_et
    pressure = ${p_initial}
    eqn = 'energy'
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    value_x = '${u_in}'
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
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_var_residual_norms = true
[]
