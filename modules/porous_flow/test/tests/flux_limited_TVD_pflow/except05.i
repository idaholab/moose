# Exception test: Dictator cannot determine the FEType and it is not properly specified in the AdvectiveFluxCalculator
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[GlobalParams]
  gravity = '1 2 3'
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
  [tracer]
  []
  [cm]
    family = Monomial
    order = constant
  []
[]

[Kernels]
  [cm]
    type = Diffusion
    variable = cm
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
  []
[]

[PorousFlowUnsaturated]
  porepressure = pp
  mass_fraction_vars = tracer
  fp = the_simple_fluid
[]

[UserObjects]
  [dummy_dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp cm'
    number_fluid_phases = 1
    number_fluid_components = 2
  []
  [advective_flux_calculator]
    type = PorousFlowAdvectiveFluxCalculatorSaturated
    PorousFlowDictator = dummy_dictator
  []
[]

[Materials]
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 2 0  0 0 3'
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]
