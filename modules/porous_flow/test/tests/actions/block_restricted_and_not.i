# This input file illustrates that the PorousFlow Joiners can correctly join block-restricted Materials, even when one PorousFlow material type (relative permeability and fluid properties in this case) is block-restricted for one phase and not block-restricted for another
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmin = 0
    xmax = 10
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '3 -1 -1'
    top_right = '6 1 1'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'p0 p1'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[Variables]
  [p0]
  []
  [p1]
  []
[]

[Kernels]
  [dot0]
    type = PorousFlowMassTimeDerivative
    variable = p0
    fluid_component = 0
  []
  [dot1]
    type = PorousFlowAdvectiveFlux
    variable = p1
    gravity = '0 0 0'
    fluid_component = 1
  []
[]

[AuxVariables]
  [m0]
  []
  [m1]
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePP
    capillary_pressure = pc
    phase0_porepressure = p0
    phase1_porepressure = p1
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'm0 m1'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [simple_fluid10]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 1
    block = 0
  []
  [simple_fluid11]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 1
    block = 1
  []
  [porosity0]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 0
  []
  [porosity1]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0 0 0  0 0 0  0 0 0'
  []
  [relperm00]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    block = 0
  []
  [relperm01]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    block = 1
  []
  [relperm1_nonblockrestricted]
    type = PorousFlowRelativePermeabilityConst
    phase = 1
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
