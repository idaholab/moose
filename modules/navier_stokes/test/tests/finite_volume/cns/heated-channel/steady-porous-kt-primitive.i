p_initial=1.01e5
T=273.15
u_in=10
eps=1
ud_in=${fparse u_in * eps}

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
  [pressure]
    type = MooseVariableFVReal
    initial_condition = ${p_initial}
  []
  [vel_x]
    type = MooseVariableFVReal
    initial_condition = ${ud_in}
  []
  [temperature]
    type = MooseVariableFVReal
    initial_condition = ${T}
  []
[]

[AuxVariables]
  [rho]
    type = MooseVariableFVReal
  []
  [rho_u]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [rho]
    type = ADMaterialRealAux
    variable = rho
    property = rho
    execute_on = 'timestep_end'
  []
  [rho_u]
    type = ADMaterialRealAux
    variable = rho_u
    property = rhou
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = pressure
    eqn = "mass"
  []

  [momentum_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = vel_x
    eqn = "momentum"
    momentum_component = 'x'
  []

  [energy_advection]
    type = PCNSFVInterpolatedLaxFriedrichs
    variable = temperature
    eqn = "energy"
  []
  [heat]
    type = FVBodyForce
    variable = temperature
    value = 1e6
  []
[]

[FVBCs]
  [rho_left]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'left'
    variable = pressure
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'mass'
  []
  [rhou_left]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'left'
    variable = vel_x
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_left]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'left'
    variable = temperature
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'energy'
  []
  [rho_right]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'right'
    variable = pressure
    p = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'right'
    variable = vel_x
    p = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_right]
    type = PCNSFVInterpolatedLaxFriedrichsBC
    boundary = 'right'
    variable = temperature
    p = ${p_initial}
    eqn = 'energy'
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    value_x = '${ud_in}'
  []
[]

[Materials]
  [var_mat]
    type = PorousPrimitiveVarMaterial
    p = pressure
    T_fluid = temperature
    superficial_vel_x = vel_x
    fp = fp
    porosity = porosity
  []
  [zero]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '${eps}'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Steady
  automatic_scaling = true
  verbose = true
[]

[Outputs]
  exodus = true
[]

[Debug]
  show_var_residual_norms = true
[]
