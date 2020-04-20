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
  [./dummy]
  [../]
[]

[AuxVariables]
  [./pressure]
    initial_condition = 2e6
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./temperature]
    initial_condition = 350
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./e_from_p_rho]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./e]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./rho]
    type = MaterialRealAux
    variable = rho
    property = density
  [../]
  [./internal_energy]
    type = MaterialRealAux
    variable = e
    property = e
  [../]
  [./internal_energy_from_p_rho]
    type = InternalEnergyAux
    variable = e_from_p_rho
    pressure = pressure
    density = rho
    fp = tabulated
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
    [./tabulated]
      type = TabulatedFluidProperties
      fp = co2
      fluid_property_file = fluid_properties.csv
      enable_T_from_p_rho = true
      # increasing num_rho leads to a smaller difference
      # in e and e_from_p_rho. With num_rho -> infinty, the
      # difference goes to zero  
      num_rho = 250
    [../]
  []
[]

[Materials]
  [./fp_mat]
    type = FluidPropertiesMaterialPT
    pressure = pressure
    temperature = temperature
    fp = tabulated
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = dummy
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Postprocessors]
  [./rho]
    type = ElementalVariableValue
    elementid = 0
    variable = rho
  [../]
  [./e]
    type = ElementalVariableValue
    elementid = 0
    variable = e
  [../]
  [./e_from_p_rho]
    type = ElementalVariableValue
    elementid = 0
    variable = e_from_p_rho
  [../]
[]

[Outputs]
  csv = true
  file_base = tabulated_out
  execute_on = 'TIMESTEP_END'
  perf_graph = true
[]
