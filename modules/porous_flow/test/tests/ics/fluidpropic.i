# Test the correct calculation of fluid properties using PorousFlowFluidPropertyIC
#
# Variables:
# Pressure: 1 MPa
# Temperature: 323.15 K
#
# Fluid properties for water (reference values from NIST webbook)
# Density: 988.43 kg/m^3
# Enthalpy: 210.19 kJ/kg
# Internal energy: 2019.18 kJ/kg

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [pressure]
    initial_condition = 1e6
  []
  [temperature]
    initial_condition = 323.15
  []
[]

[AuxVariables]
  [enthalpy]
  []
  [internal_energy]
  []
  [density]
  []
[]

[ICs]
  [enthalpy]
    type = PorousFlowFluidPropertyIC
    variable = enthalpy
    property = enthalpy
    porepressure = pressure
    temperature = temperature
    fp = water
  []
  [internal_energy]
    type = PorousFlowFluidPropertyIC
    variable = internal_energy
    property = internal_energy
    porepressure = pressure
    temperature = temperature
    fp = water
  []
  [density]
    type = PorousFlowFluidPropertyIC
    variable = density
    property = density
    porepressure = pressure
    temperature = temperature
    fp = water
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[Kernels]
  [pressure]
    type = Diffusion
    variable = pressure
  []
  [temperature]
    type = Diffusion
    variable = temperature
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [enthalpy]
    type = ElementAverageValue
    variable = enthalpy
    execute_on = 'initial timestep_end'
  []
  [internal_energy]
    type = ElementAverageValue
    variable = internal_energy
    execute_on = 'initial timestep_end'
  []
  [density]
    type = ElementAverageValue
    variable = density
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  execute_on = initial
[]
