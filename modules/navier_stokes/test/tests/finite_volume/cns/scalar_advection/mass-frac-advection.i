rho_initial=1.29
p_initial=1.01e5
T=273.15
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
v_in=1

[GlobalParams]
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    nx = 2
    ymin = 0
    ymax = 10
    ny = 20
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
  [rho_v]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
  [rho_et]
    type = MooseVariableFVReal
    initial_condition = ${rho_et_initial}
    scaling = 1e-5
  []
  [mass_frac]
    type = MooseVariableFVReal
    initial_condition = 1e-15
  []
[]

[AuxVariables]
  [U_x]
    type = MooseVariableFVReal
  []
  [U_y]
    type = MooseVariableFVReal
  []
  [pressure]
    type = MooseVariableFVReal
  []
  [temperature]
    type = MooseVariableFVReal
  []
  [courant]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [U_x]
    type = ADMaterialRealAux
    variable = U_x
    property = vel_x
    execute_on = 'timestep_end'
  []
  [U_y]
    type = ADMaterialRealAux
    variable = U_y
    property = vel_y
    execute_on = 'timestep_end'
  []
  [pressure]
    type = ADMaterialRealAux
    variable = pressure
    property = p
    execute_on = 'timestep_end'
  []
  [temperature]
    type = ADMaterialRealAux
    variable = temperature
    property = T_fluid
    execute_on = 'timestep_end'
  []
  [courant]
    type = Courant
    variable = courant
    u = U_x
    v = U_y
  []
[]

[FVKernels]
  [mass_time]
    type = FVPorosityTimeDerivative
    variable = rho
  []
  [mass_advection]
    type = PCNSFVLaxFriedrichs
    variable = rho
    eqn = "mass"
  []

  [momentum_time_x]
    type = FVTimeKernel
    variable = rho_u
  []
  [momentum_advection_and_pressure_x]
    type = PCNSFVLaxFriedrichs
    variable = rho_u
    eqn = "momentum"
    momentum_component = 'x'
  []

  [momentum_time_y]
    type = FVTimeKernel
    variable = rho_v
  []
  [momentum_advection_and_pressure_y]
    type = PCNSFVLaxFriedrichs
    variable = rho_v
    eqn = "momentum"
    momentum_component = 'y'
  []

  [energy_time]
    type = FVPorosityTimeDerivative
    variable = rho_et
  []
  [energy_advection]
    type = PCNSFVLaxFriedrichs
    variable = rho_et
    eqn = "energy"
  []

  [mass_frac_time]
    type = PCNSFVDensityTimeDerivative
    variable = mass_frac
    rho = rho
  []
  [mass_frac_advection]
    type = PCNSFVLaxFriedrichs
    variable = mass_frac
    eqn = "scalar"
  []
[]

[Functions]
  [ud_in]
    type = ParsedVectorFunction
    value_x = '0'
    value_y = '${v_in}'
  []
[]


[FVBCs]
  [rho_bottom]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'bottom'
    variable = rho
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'mass'
  []
  [rho_u_bottom]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'bottom'
    variable = rho_u
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_v_bottom]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'bottom'
    variable = rho_v
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'momentum'
    momentum_component = 'y'
  []
  [rho_et_bottom]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'bottom'
    variable = rho_et
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    eqn = 'energy'
  []
  [mass_frac_bottom]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'bottom'
    variable = mass_frac
    superficial_velocity = 'ud_in'
    T_fluid = ${T}
    scalar = 1
    eqn = 'scalar'
  []

  [rho_top]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'top'
    variable = rho
    p = ${p_initial}
    eqn = 'mass'
  []
  [rho_u_top]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'top'
    variable = rho_u
    p = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_v_top]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'top'
    variable = rho_v
    p = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'y'
  []
  [rho_et_top]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'top'
    variable = rho_et
    p = ${p_initial}
    eqn = 'energy'
  []
  [mass_frac_top]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'top'
    variable = mass_frac
    p = ${p_initial}
    eqn = 'scalar'
  []

  [momentum_x_walls]
    type = PCNSFVImplicitMomentumPressureBC
    variable = rho_u
    boundary = 'left right'
    momentum_component = 'x'
  []
  [momentum_y_walls]
    type = PCNSFVImplicitMomentumPressureBC
    variable = rho_v
    boundary = 'left right'
    momentum_component = 'y'
  []
[]

[Materials]
  [var_mat]
    type = PorousConservedVarMaterial
    rho = rho
    rho_et = rho_et
    superficial_rhou = rho_u
    superficial_rhov = rho_v
    fp = fp
    porosity = porosity
  []
  [porosity]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  nl_max_its = 10
  [TimeIntegrator]
    type = ActuallyExplicitEuler
  []
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  abort_on_solve_fail = true
  dt = 5e-4
  num_steps = 25
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'initial timestep_end'
  []
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
