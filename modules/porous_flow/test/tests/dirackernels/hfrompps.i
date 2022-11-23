[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pressure]
  []

  [temperature]
    scaling = 1E-6
  []
[]

[ICs]
  [pressure_ic]
    type = ConstantIC
    variable = pressure
    value = 1e6
  []
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = 400
  []
[]

[Kernels]
  [P_time_deriv]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pressure
  []

  [P_flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pressure
    gravity = '0 -9.8 0'
  []

  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
 []

  [heat_conduction]
    type = PorousFlowHeatConduction
    variable = temperature
  []

  [heat_advection]
    type = PorousFlowHeatAdvection
    variable = temperature
    gravity = '0 -9.8 0'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pressure temperature'
    number_fluid_phases = 1
    number_fluid_components = 1
  []

  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[Functions]
  [mass_flux_in_fn]
    type = PiecewiseConstant
    direction = left
    xy_data = '
      0    0
      100  0.1
      300  0
      600  0.1
      1400 0
      1500 0.2'
  []

  [T_in_fn]
    type = PiecewiseLinear
    xy_data = '
      0    400
      600  450'
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
    density0 = 1000
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []

  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pressure
    capillary_pressure = pc
  []

  [massfrac]
    type = PorousFlowMassFraction
    at_nodes = true
  []

  [fluid_props]
    type = PorousFlowSingleComponentFluid
    phase = 0
    fp = simple_fluid
  []

  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 1
    phase = 0
  []

  [fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = simple_fluid
  []

  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 830.0
    density = 2750
  []

  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '2.5 0 0  0 2.5 0  0 0 2.5'
  []

  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.0E-15 0 0  0 1.0E-15 0  0 0 1.0E-14'
  []

  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowPointSourceFromPostprocessor
    variable = pressure
    mass_flux = mass_flux_in
    point = '0.5 0.5 0'
  []
  [source_h]
    type = PorousFlowPointEnthalpySourceFromPostprocessor
    variable = temperature
    mass_flux = mass_flux_in
    point = '0.5 0.5 0'
    T_in = T_in
    pressure = pressure
    fp = simple_fluid
  []
[]

[Preconditioning]
  [preferred]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = ' lu     '
  []
[]

[Postprocessors]
  [total_mass]
    type = PorousFlowFluidMass
    execute_on = 'initial timestep_end'
  []

  [total_heat]
    type = PorousFlowHeatEnergy
  []

  [mass_flux_in]
    type = FunctionValuePostprocessor
    function = mass_flux_in_fn
    execute_on = 'initial timestep_end'
  []

  [avg_temp]
    type = ElementAverageValue
    variable = temperature
    execute_on = 'initial timestep_end'
  []

  [T_in]
    type = FunctionValuePostprocessor
    function = T_in_fn
    execute_on = 'initial timestep_end'
  []
[]


[Executioner]
  type = Transient
  solve_type = Newton
  nl_abs_tol = 1e-14
  dt = 100
  end_time = 2000
[]

[Outputs]
  csv = true
  execute_on = 'initial timestep_end'
  file_base = hfrompps
[]
