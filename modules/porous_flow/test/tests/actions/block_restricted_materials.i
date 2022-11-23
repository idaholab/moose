# Tests that the actions to automatically add PorousFlowJoiner's and the correct
# qp or nodal version of each material work as expected when a material is block
# restricted. Tests both phase dependent properties (like relative permeability)
# as well as phase-independent materials (like porosity)

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    ny = 2
  []
  [subdomain0]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 0.5 0'
    block_id = 0
  []
  [subdomain1]
    input = subdomain0
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0.5 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Variables]
  [p0]
    initial_condition = 1
  []
  [p1]
    initial_condition = 1.1
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
  [kl]
    family = MONOMIAL
    order = CONSTANT
  []
  [kg]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [porosity]
    type = PorousFlowPropertyAux
    property = porosity
    variable = porosity
  []
  [kl]
    type = PorousFlowPropertyAux
    property = relperm
    variable = kl
    phase = 0
  []
  [kg]
    type = PorousFlowPropertyAux
    property = relperm
    variable = kg
    phase = 1
  []
[]

[Kernels]
  [p0]
    type = PorousFlowMassTimeDerivative
    variable = p0
  []
  [p1]
    type = PorousFlowAdvectiveFlux
    gravity = '0 0 0'
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
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = p0
    phase1_porepressure = p1
    capillary_pressure = pc
  []
  [krl0]
    type = PorousFlowRelativePermeabilityConst
    kr = 0.7
    phase = 0
    block = 0
  []
  [krg0]
    type = PorousFlowRelativePermeabilityConst
    kr = 0.8
    phase = 1
    block = 0
  []
  [krl1]
    type = PorousFlowRelativePermeabilityConst
    kr = 0.5
    phase = 0
    block = 1
  []
  [krg1]
    type = PorousFlowRelativePermeabilityConst
    kr = 0.4
    phase = 1
    block = 1
  []
  [perm]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 1 0 0 0 1'
  []
  [fluid0]
    type = PorousFlowSingleComponentFluid
    fp = fluid0
    phase = 0
  []
  [fluid1]
    type = PorousFlowSingleComponentFluid
    fp = fluid1
    phase = 1
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [porosity0]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 0
  []
  [porosity1]
    type = PorousFlowPorosityConst
    porosity = 0.2
    block = 1
  []
[]


[Executioner]
  type = Transient
  end_time = 1
  nl_abs_tol = 1e-10
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

[Outputs]
  exodus = true
[]
