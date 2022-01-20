# Test thermophysical property calculations using TabulatedFluidProperties.
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
      type = CO2FluidProperties
    []
    [tabulated]
      type = TabulatedFluidProperties
      fp = co2
      construct_pT_from_ve = true
      fluid_property_file = fluid_properties.csv
      error_on_out_of_bounds = false
      num_v = 4
      num_e = 4
      num_p = 4
      num_T = 4
    []
  []
[]

[Materials]
  [fp_mat_ve]
    type = FluidPropertiesMaterial
    v = 0.0310899837399385
    e = -3.079760e+04
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
  file_base = tabulated_v_e_out
  execute_on = 'TIMESTEP_END'
  perf_graph = true
[]
