rho_initial=1.29
p_initial=1.01e5
T=273.15
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# No bulk velocity in the domain initially
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
# u refers to the superficial velocity
u_in=1

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
  [porosity]
    type = MooseVariableFVReal
  []
  [real_vel_x]
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
  [porosity]
    type = MaterialRealAux
    variable = porosity
    property = porosity
    execute_on = 'timestep_end'
  []
  [real_vel_x]
    type = ParsedAux
    variable = real_vel_x
    function = 'vel_x / porosity'
    args = 'vel_x porosity'
    execute_on = 'timestep_end'
  []
  [specific_internal_energy]
    type = ParsedAux
    variable = specific_internal_energy
    function = 'rho_et / rho - (real_vel_x * real_vel_x) / 2'
    args = 'rho_et rho real_vel_x'
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
    vel_x = real_vel_x
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
    function = 'vel_x * rho_u / porosity + pressure * porosity'
    args = 'vel_x rho_u porosity pressure'
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
    u = real_vel_x
    execute_on = 'timestep_end'
  []
  [worst_courant]
    type = Courant
    variable = worst_courant
    u = real_vel_x
    execute_on = 'timestep_end'
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

  [momentum_time]
    type = FVTimeKernel
    variable = rho_u
  []
  [momentum_advection]
    type = PCNSFVLaxFriedrichs
    variable = rho_u
    eqn = "momentum"
    momentum_component = 'x'
  []
  [friction]
    type = FVReaction
    variable = rho_u
    rate = 1000
  []
  [eps_grad]
    type = PNSFVEpsilonJumpsFluxKernel
    variable = rho_u
    momentum_component = 'x'
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
    p = ${p_initial}
    eqn = 'mass'
  []
  [rhou_right]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'right'
    variable = rho_u
    p = ${p_initial}
    eqn = 'momentum'
    momentum_component = 'x'
  []
  [rho_et_right]
    type = PCNSFVLaxFriedrichsBC
    boundary = 'right'
    variable = rho_et
    p = ${p_initial}
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
    block = 0
  []
  [one]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 1
  []
  [two]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.25'
    block = 2
  []
  [three]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '0.5'
    block = 3
  []
  [four]
    type = GenericConstantMaterial
    prop_names = 'porosity'
    prop_values = '1'
    block = 4
  []
[]

[Executioner]
  solve_type = NEWTON
  type = Transient
  nl_max_its = 10
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 5e-5
    optimal_iterations = 6
  []
  steady_state_detection = true
  steady_state_tolerance = 1e-12
  abort_on_solve_fail = true
  num_steps = 250
  nl_abs_tol = 1e-9
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
