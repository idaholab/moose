# Test thermophysical property calculations using TabulatedBiCubic/LinearFluidProperties.
# Calculations for density, internal energy and enthalpy using bicubic or bilinear
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
  [p]
    family = MONOMIAL
    order = CONSTANT
  []
  [T]
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
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [g]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [viscosity]
    type = MaterialRealAux
    variable = mu
    property = mu
  []
  [thermal_conductivity]
    type = MaterialRealAux
    variable = k
    property = k
  []
  [pressure]
    type = MaterialRealAux
    variable = p
    property = pressure
  []
  [temperature]
    type = MaterialRealAux
    variable = T
    property = temperature
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
  [g]
    type = MaterialRealAux
    variable = g
    property = g
  []
[]

[Modules]
  [FluidProperties]
    [co2]
      type = IdealGasFluidProperties
    []
    [tabulated]
      type = TabulatedBilinearFluidProperties
      # type = TabulatedBicubicFluidProperties
      # fp = co2
      interpolated_properties = 'density enthalpy viscosity internal_energy k c cv cp entropy'
      # fluid_property_file = fluid_properties.csv
      save_file = true
      construct_pT_from_ve = true
      construct_pT_from_vh = true
      error_on_out_of_bounds = false

      # Tabulation range
      temperature_min = 280
      temperature_max = 600
      pressure_min = 1e5
      pressure_max = 7e5

      # Newton parameters
      tolerance = 1e-8
      T_initial_guess = 310
      p_initial_guess = 1.8e5
    []
  []
[]

[Materials]
  [fp_mat_ve]
    type = FluidPropertiesMaterial
    v = 0.4957
    e = 310163
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

[Postprocessors]
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
  [p]
    type = ElementalVariableValue
    elementid = 0
    variable = p
  []
  [T]
    type = ElementalVariableValue
    elementid = 0
    variable = T
  []
  [k]
    type = ElementalVariableValue
    elementid = 0
    variable = k
  []
  [g]
    type = ElementalVariableValue
    elementid = 0
    variable = g
  []
[]

[Outputs]
  csv = true
  file_base = tabulated_v_e_bilinear_out
  # file_base = tabulated_v_e_out
  execute_on = 'TIMESTEP_END'
  perf_graph = true
[]
