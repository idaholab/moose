# Test thermophysical property calculations using TabulatedBiCubic/LinearFluidProperties.
# Calculations for density, internal energy and enthalpy using bicubic spline
# interpolation of data generated using CO2FluidProperties.

[Mesh]
  type = GeneratedMesh
  dim = 2
  # This test uses ElementalVariableValue postprocessors on specific
  # elements, so element numbering needs to stay unchanged
  allow_renumbering = false
[]

[Variables]
  [dummy]
  []
[]

[AuxVariables]
  [pressure]
    initial_condition = 2e6
    family = MONOMIAL
    order = CONSTANT
  []
  [temperature]
    initial_condition = 350
    family = MONOMIAL
    order = CONSTANT
  []
  [rho]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu]
    family = MONOMIAL
    order = CONSTANT
  []
  [e]
    family = MONOMIAL
    order = CONSTANT
  []
  [h]
    family = MONOMIAL
    order = CONSTANT
  []
  [s]
    family = MONOMIAL
    order = CONSTANT
  []
  [cv]
    family = MONOMIAL
    order = CONSTANT
  []
  [cp]
    family = MONOMIAL
    order = CONSTANT
  []
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [rho]
    type = MaterialRealAux
    variable = rho
    property = density
  []
  [my]
    type = MaterialRealAux
    variable = mu
    property = viscosity
  []
  [internal_energy]
    type = MaterialRealAux
    variable = e
    property = e
  []
  [enthalpy]
    type = MaterialRealAux
    variable = h
    property = h
  []
  [entropy]
    type = MaterialRealAux
    variable = s
    property = s
  []
  [cv]
    type = MaterialRealAux
    variable = cv
    property = cv
  []
  [cp]
    type = MaterialRealAux
    variable = cp
    property = cp
  []
  [c]
    type = MaterialRealAux
    variable = c
    property = c
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [tabulated]
    type = TabulatedBicubicFluidProperties
    fp = co2
    interpolated_properties = 'density enthalpy viscosity internal_energy k c cv cp entropy'
    # fluid_property_file = fluid_properties.csv
    construct_pT_from_ve = false
    construct_pT_from_vh = false

    # Tabulation range
    temperature_min = 280
    temperature_max = 600
    pressure_min = 1e5
    pressure_max = 3e6

    # Newton parameters
    tolerance = 1e-8
    T_initial_guess = 350
    p_initial_guess = 1.5e5
  []
[]

[Materials]
  [fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = tabulated
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = dummy
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [rho]
    type = ElementalVariableValue
    elementid = 0
    variable = rho
  []
  [mu]
    type = ElementalVariableValue
    elementid = 0
    variable = mu
  []
  [e]
    type = ElementalVariableValue
    elementid = 0
    variable = e
  []
  [h]
    type = ElementalVariableValue
    elementid = 0
    variable = h
  []
  [s]
    type = ElementalVariableValue
    elementid = 0
    variable = s
  []
  [cv]
    type = ElementalVariableValue
    elementid = 0
    variable = cv
  []
  [cp]
    type = ElementalVariableValue
    elementid = 0
    variable = cp
  []
  [c]
    type = ElementalVariableValue
    elementid = 0
    variable = c
  []
[]

[Outputs]
  csv = true
  file_base = tabulated_out
  execute_on = 'TIMESTEP_END'
  perf_graph = true
[]
