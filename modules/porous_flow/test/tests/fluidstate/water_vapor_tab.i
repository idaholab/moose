# Tests correct calculation of properties in PorousFlowWaterVapor in the two-phase region

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pliq]
    initial_condition = 1e6
  []
  [h]
    initial_condition = 8e5
    scaling = 1e-3
  []
[]

[AuxVariables]
  [pressure_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [pressure_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_gas]
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
  [temperature]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [enthalpy_water]
    type = PorousFlowPropertyAux
    variable = enthalpy_water
    property = enthalpy
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [enthalpy_gas]
    type = PorousFlowPropertyAux
    variable = enthalpy_gas
    property = enthalpy
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [pressure_water]
    type = PorousFlowPropertyAux
    variable = pressure_water
    property = pressure
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [pressure_gas]
    type = PorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [saturation_water]
    type = PorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [density_water]
    type = PorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [viscosity_water]
    type = PorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [temperature]
    type = PorousFlowPropertyAux
    variable = temperature
    property = temperature
    execute_on = 'initial timestep_end'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pliq h'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureBC
    pe = 1e5
    lambda = 2
    pc_max = 1e6
  []
  [fs]
    type = PorousFlowWaterVapor
    water_fp = water
    capillary_pressure = pc
  []
[]

[FluidProperties]

  [water_true]
    type = Water97FluidProperties
  []
  [water]
    type = TabulatedBicubicFluidProperties
    fp = water_true
    allow_fp_and_tabulation = true
    fluid_property_file = fluid_properties_extended.csv
  []
[]

[Materials]
  [watervapor]
    type = PorousFlowFluidStateSingleComponent
    porepressure = pliq
    enthalpy = h
    temperature_unit = Kelvin
    capillary_pressure = pc
    fluid_state = fs
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-13 0 0 0 1e-13 0 0 0 1e-13'
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
  [internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 2500
    specific_heat_capacity = 1200
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [density_water]
    type = ElementAverageValue
    variable = density_water
    execute_on = 'initial timestep_end'
  []
  [density_gas]
    type = ElementAverageValue
    variable = density_gas
    execute_on = 'initial timestep_end'
  []
  [viscosity_water]
    type = ElementAverageValue
    variable = viscosity_water
    execute_on = 'initial timestep_end'
  []
  [viscosity_gas]
    type = ElementAverageValue
    variable = viscosity_gas
    execute_on = 'initial timestep_end'
  []
  [enthalpy_water]
    type = ElementAverageValue
    variable = enthalpy_water
    execute_on = 'initial timestep_end'
  []
  [enthalpy_gas]
    type = ElementAverageValue
    variable = enthalpy_gas
    execute_on = 'initial timestep_end'
  []
  [sg]
    type = ElementAverageValue
    variable = saturation_gas
    execute_on = 'initial timestep_end'
  []
  [sw]
    type = ElementAverageValue
    variable = saturation_water
    execute_on = 'initial timestep_end'
  []
  [pwater]
    type = ElementAverageValue
    variable = pressure_water
    execute_on = 'initial timestep_end'
  []
  [pgas]
    type = ElementAverageValue
    variable = pressure_gas
    execute_on = 'initial timestep_end'
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
    execute_on = 'initial timestep_end'
  []
  [enthalpy]
    type = ElementAverageValue
    variable = h
    execute_on = 'initial timestep_end'
  []
  [liquid_mass]
    type = PorousFlowFluidMass
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [vapor_mass]
    type = PorousFlowFluidMass
    phase = 1
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  file_base = water_vapor_twophase_tab
  csv = true
  execute_on = INITIAL
[]
