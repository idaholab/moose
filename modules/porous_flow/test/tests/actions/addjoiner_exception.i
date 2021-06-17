# Tests that including a PorousFlowJoiner material throws the
# informative deprecation warning rather than a duplicate material property error

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

[Materials]
  [temperature]
    type = PorousFlowTemperature
    at_nodes = true
  []
  [temperature_qp]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    at_nodes = true
    phase0_porepressure = p0
    phase1_porepressure = p1
    capillary_pressure = pc
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityConst
    at_nodes = true
    kr = 0.5
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityConst
    at_nodes = true
    kr = 0.8
    phase = 1
  []
  [relperm]
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
