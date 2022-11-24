# Example of accessing properties using the PorousFlowPropertyAux AuxKernel for
# each phase and fluid component (as required).

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pwater]
    initial_condition = 1e6
  []
  [sgas]
    initial_condition = 0.3
  []
  [temperature]
    initial_condition = 50
  []
[]

[AuxVariables]
  [x0_water]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  []
  [x0_gas]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.8
  []
  [pressure_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [capillary_pressure]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [relperm_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [relperm_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [energy_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [energy_gas]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [pressure_gas]
    type = PorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = timestep_end
  []
  [capillary_pressure]
    type = PorousFlowPropertyAux
    variable = capillary_pressure
    property = capillary_pressure
    execute_on = timestep_end
  []
  [saturation_water]
    type = PorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = timestep_end
  []
  [density_water]
    type = PorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = timestep_end
  []
  [density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = timestep_end
  []
  [viscosity_water]
    type = PorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = timestep_end
  []
  [viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = timestep_end
  []
  [relperm_water]
    type = PorousFlowPropertyAux
    variable = relperm_water
    property = relperm
    phase = 0
    execute_on = timestep_end
  []
  [relperm_gas]
    type = PorousFlowPropertyAux
    variable = relperm_gas
    property = relperm
    phase = 1
    execute_on = timestep_end
  []
  [x1_water]
    type = PorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = timestep_end
  []
  [x1_gas]
    type = PorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = timestep_end
  []
  [enthalpy_water]
    type = PorousFlowPropertyAux
    variable = enthalpy_water
    property = enthalpy
    phase = 0
    execute_on = timestep_end
  []
  [enthalpy_gas]
    type = PorousFlowPropertyAux
    variable = enthalpy_gas
    property = enthalpy
    phase = 1
    execute_on = timestep_end
  []
  [energy_water]
    type = PorousFlowPropertyAux
    variable = energy_water
    property = internal_energy
    phase = 0
    execute_on = timestep_end
  []
  [energy_gas]
    type = PorousFlowPropertyAux
    variable = energy_gas
    property = internal_energy
    phase = 1
    execute_on = timestep_end
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  []
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
  []
  [heat_advection]
    type = PorousFlowHeatAdvection
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater sgas temperature'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-5
    pc_max = 1e7
    sat_lr = 0.1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1e9
    viscosity = 1e-3
    density0 = 1000
    thermal_expansion = 0
    cv = 2
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 1e9
    viscosity = 1e-4
    density0 = 20
    thermal_expansion = 0
    cv = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = pwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'x0_water x0_gas'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []

  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 125
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
[]
