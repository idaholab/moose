# Tests that including PorousFlowJoiner materials doesn't cause the simulation
# to fail due to the PorousFlowAddMaterialJoiner action adding duplicate
# PorousFlowJoiner materials

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [p0]
  []
  [p1]
  []
[]

[Kernels]
  [p0]
    type = Diffusion
    variable = p0
  []
  [p1]
    type = Diffusion
    variable = p1
  []
[]

[FluidProperties]
  [fluid0]
    type = SimpleFluidProperties
  []
  [fluid1]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    at_nodes = true
  []
  [temperature_qp]
    type = PorousFlowTemperature
  []
  [ppss_nodal]
    type = PorousFlow2PhasePP
    at_nodes = true
    phase0_porepressure = p0
    phase1_porepressure = p1
    capillary_pressure = pc
  []
  [ppss_qp]
    type = PorousFlow2PhasePP
    phase0_porepressure = p0
    phase1_porepressure = p1
    capillary_pressure = pc
  []
  [fluid0_nodal]
    type = PorousFlowSingleComponentFluid
    fp = fluid0
    at_nodes = true
    phase = 0
  []
  [fluid1_nodal]
    type = PorousFlowSingleComponentFluid
    fp = fluid1
    at_nodes = true
    phase = 1
  []
  [fluid0_qp]
    type = PorousFlowSingleComponentFluid
    fp = fluid0
    phase = 0
  []
  [fluid1_qp]
    type = PorousFlowSingleComponentFluid
    fp = fluid1
    phase = 1
  []
  [density_nodal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_density_nodal
  []
  [density_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_density_qp
  []
  [viscosity_nodal]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_nodal
    at_nodes = true
  []
  [viscosity_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  []
  [energy_ndoal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_fluid_phase_internal_energy_nodal
  []
  [energy_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_internal_energy_qp
  []
  [enthalpy_nodal]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_enthalpy_nodal
    at_nodes = true
  []
  [enthalpy_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_fluid_phase_enthalpy_qp
  []
  [relperm0_nodal]
    type = PorousFlowRelativePermeabilityConst
    at_nodes = true
    kr = 0.5
    phase = 0
  []
  [relperm1_nodal]
    type = PorousFlowRelativePermeabilityConst
    at_nodes = true
    kr = 0.8
    phase = 1
  []
  [relperm_nodal]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  []
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'p0 p1'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
[]
