rho_initial=6.335
p_initial=7e6
gamma=1.4
e_initial=${fparse p_initial / (gamma - 1) / rho_initial}
# No bulk velocity in the domain initially
et_initial=${e_initial}
rho_et_initial=${fparse rho_initial * et_initial}
# prescribe inlet rho = initial rho
rho_in=${rho_initial}
# u refers to the superficial velocity
u_in=2.37
mass_flux_in=${fparse u_in * rho_in}

[GlobalParams]
  flux_interp_method = 'average'
  interp_method = 'average'
  fp = fp
[]

[Mesh]
  [cartesian]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 10
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
  [one_to_pt5]
    type = SideSetsBetweenSubdomainsGenerator
    input = one_again
    primary_block = 0
    paired_block = 1
    new_boundary = one_to_pt5
  []
  [pt5_to_pt25]
    type = SideSetsBetweenSubdomainsGenerator
    input = one_to_pt5
    primary_block = 1
    paired_block = 2
    new_boundary = pt5_to_pt25
  []
  [pt25_to_pt5]
    type = SideSetsBetweenSubdomainsGenerator
    input = pt5_to_pt25
    primary_block = 2
    paired_block = 3
    new_boundary = pt25_to_pt5
  []
  [pt5_to_one]
    type = SideSetsBetweenSubdomainsGenerator
    input = pt25_to_pt5
    primary_block = 3
    paired_block = 4
    new_boundary = pt5_to_one
  []
[]

[Modules]
  [FluidProperties]
    [fp]
      type = HeliumFluidProperties
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
    # initial_condition = 1e-15
    initial_condition = ${mass_flux_in}
  []
  [rho_et]
      type = MooseVariableFVReal
    initial_condition = ${rho_et_initial}
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
  # [eps_grad]
  #   type = PNSFVPGradEpsilon
  #   variable = rho_u
  #   momentum_component = 'x'
  #   epsilon_function = 'epsilon_function'
  # []

  [energy_advection]
    type = NSFVMassFluxAdvection
    variable = rho_et
    advected_quantity = 'ht'
  []
[]

[FVInterfaceKernels]
  [one_to_pt5]
    type = FVEpsilonJumps
    variable1 = 'rho_u'
    boundary = 'one_to_pt5'
    subdomain1 = '0'
    subdomain2 = '1'
    momentum_component = 'x'
  []
  [pt5_to_pt25]
    type = FVEpsilonJumps
    variable1 = 'rho_u'
    boundary = 'pt5_to_pt25'
    subdomain1 = '1'
    subdomain2 = '2'
    momentum_component = 'x'
  []
  [pt25_to_pt5]
    type = FVEpsilonJumps
    variable1 = 'rho_u'
    boundary = 'pt25_to_pt5'
    subdomain1 = '2'
    subdomain2 = '3'
    momentum_component = 'x'
  []
  [pt5_to_one]
    type = FVEpsilonJumps
    variable1 = 'rho_u'
    boundary = 'pt5_to_one'
    subdomain1 = '3'
    subdomain2 = '4'
    momentum_component = 'x'
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
    p = ${p_initial}
  []
  [rho_et_left]
    type = PNSFVFluidEnergySpecifiedTemperatureBC
    variable = rho_et
    boundary = 'left'
    superficial_rhou = ${mass_flux_in}
    temperature = 523
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
  # [porosity]
  #   type = GenericFunctionMaterial
  #   prop_names = 'porosity'
  #   prop_values = 'epsilon_function'
  # []
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
  type = Steady
  automatic_scaling = true
  verbose = true
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
  nl_max_its = 10
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
