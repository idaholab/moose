# Test the density, viscosity, enthalpy and internal energy
# calculated by the PorousFlowBrine material when using
# PorousFlowFullySaturated action.
# Density (rho) and enthalpy (h) From Driesner (2007), Geochimica et
# Cosmochimica Acta 71, 4902-4919 (2007).
# Viscosity from Phillips et al, A technical databook for
# geothermal energy utilization, LbL-12810 (1981).
# Internal energy = h - p / rho.
# Pressure 20 MPa
# Temperature 50C
# xnacl = 0.1047 (equivalent to 2.0 molality)

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  block = '0'
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[PorousFlowFullySaturated]
  coupling_type = ThermoHydro
  porepressure = pp
  temperature = temp
  mass_fraction_vars = "nacl"
  fluid_properties_type = PorousFlowBrine
  nacl_name = nacl
  dictator_name = dictator
  stabilization = none
[]

[Variables]
  [pp]
    initial_condition = 20E6
  []
  [temp]
    initial_condition = 323.15
  []
  [nacl]
    initial_condition = 0.1047
  []
[]

[Kernels]
  # All provided by PorousFlowFullySaturated action
[]

[BCs]
  [t_bdy]
    type = DirichletBC
    variable = temp
    boundary = 'left right'
    value = 323.15
  []
  [p_bdy]
    type = DirichletBC
    variable = pp
    boundary = 'left right'
    value = 20E6
  []
  [nacl_bdy]
    type = DirichletBC
    variable = nacl
    boundary = 'left right'
    value = 0.1047
  []
[]

[Postprocessors]
  [pressure]
    type = ElementIntegralVariablePostprocessor
    variable = pp
  []
  [temperature]
    type = ElementIntegralVariablePostprocessor
    variable = temp
  []
  [xnacl]
    type = ElementIntegralVariablePostprocessor
    variable = nacl
  []
  [density]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_density_qp0'
  []
  [viscosity]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_viscosity_qp0'
  []
  [enthalpy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_enthalpy_qp0'
  []
  [energy]
    type = ElementIntegralMaterialProperty
    mat_prop = 'PorousFlow_fluid_phase_internal_energy_nodal0'
  []
[]

[Materials]
  # Thermal conductivity
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
    wet_thermal_conductivity = '3 0 0  0 3 0  0 0 3'
  []

  # Specific heat capacity
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 850
    density = 2700
  []

  # Permeability
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-13 0 0  0 1E-13 0  0 0 1E-13'
  []

  # Porosity
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.3
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
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  file_base = fullsat_brine
  csv = true
  execute_on = 'timestep_end'
[]
