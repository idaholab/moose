# Version of simple_fluid_yr_MPa_C.i but using a PorousFlowFullySaturated Action, to check that the Action passes the unit choices through to the remainder of PorousFlow

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 2.0E-4
    cv = 4000.0
    cp = 5000.0
    bulk_modulus = 1.0E9
    thermal_conductivity = 1.0
    viscosity = 1.1E-3
    density0 = 1500.0
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pp]
    initial_condition = 10
  []
  [T]
    initial_condition = 26.85
  []
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = pp
  temperature = T
  temperature_unit = Celsius
  pressure_unit = MPa
  time_unit = years
  fp = the_simple_fluid
[]

[Materials]
  # these are needed by the Kernels, but are irrelevant to this particular problem
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.2
  []
  [zero_thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '0 0 0  0 0 0  0 0 0'
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 1.0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0 0 0 0 0 0 0 0 0'
  []
[]

[Postprocessors]
  [pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = T
  []
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
  [internal_energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_nodal0'
  []
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_nodal0'
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  solve_type = Newton
[]

[Outputs]
  file_base = simple_fluid_yr_MPa_C_out
  execute_on = 'timestep_end'
  csv = true
[]
