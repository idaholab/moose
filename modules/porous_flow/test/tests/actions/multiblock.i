# This input file illustrates that PorousFlow can be block-restricted.  That is, porous-flow physics acts only on some blocks (block = '0, 1', in this case), and different physics, in this case diffusion, acts on other blocks (block = 2, in this case).
# Here:
# - the Variable "pressure" exists everywhere, but is governed by PorousFlow only on block = '0 1', and diffusion on block = 2
# - the Variable "temp" exists only on block = '0 1', and is governed by PorousFlow there
# - the Variable "temp1" exists only on block = 2, and is governed by diffusion there
# Hence, the PorousFlow Materials only need to be defined on block = '0 1'
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
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    block_id = 2
    bottom_left = '6 -1 -1'
    top_right = '10 1 1'
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pressure temp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[Variables]
  [pressure] # exists over the entire mesh: governed by PorousFlow on block=0, 1, and diffusion on block=2
  []
  [temp]
    block = '0 1' # only governed by PorousFlow
  []
  [temp1]
    block = 2 # only governed by diffusion
  []
[]

[Kernels]
  [porous_flow_time_derivative]
    type = PorousFlowMassTimeDerivative
    block = '0 1'
    variable = pressure
  []
  [porous_flow_flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = pressure
    block = '0 1'
  []
  [porous_flow_heat_time_derivative]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
    block = '0 1'
  []
  [porous_flow_heat_advection]
    type = PorousFlowHeatAdvection
    gravity = '0 0 0'
    variable = temp
    block = '0 1'
  []
  [diffusion_p]
    type = Diffusion
    variable = pressure
    block = 2
  []
  [diffusion_t1]
    type = Diffusion
    variable = temp1
    block = 2
  []
[]

[AuxVariables]
  [density]
    family = MONOMIAL
    order = CONSTANT
    block = '0 1'
  []
  [relperm]
    family = MONOMIAL
    order = CONSTANT
    block = '0 1'
  []
[]

[AuxKernels]
  [density]
    type = PorousFlowPropertyAux
    variable = density
    property = density
  []
  [relperm]
    type = PorousFlowPropertyAux
    variable = relperm
    property = relperm
  []
[]

[Postprocessors]
  [density1000]
    type = PointValue
    point = '0 0 0'
    variable = density
  []
  [density2000]
    type = PointValue
    point = '5 0 0'
    variable = density
  []
  [relperm0.25]
    type = PointValue
    point = '0 0 0'
    variable = relperm
  []
  [relperm0.5]
    type = PointValue
    point = '5 0 0'
    variable = relperm
  []
[]

[FluidProperties]
  [simple_fluid1000]
    type = SimpleFluidProperties
  []
  [simple_fluid2000]
    type = SimpleFluidProperties
    density0 = 2000
  []
[]

[Materials] # note these PorousFlow materials are all on block = '0 1'
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
    block = '0 1'
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
    block = '0 1'
  []
  [massfrac]
    type = PorousFlowMassFraction
    block = '0 1'
  []
  [simple_fluid1000]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1000
    phase = 0
    block = 0
  []
  [simple_fluid2000]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid2000
    phase = 0
    block = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = '0 1'
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
    block = '0 1'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    block = 0
    kr = 0.25
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    block = 1
    kr = 0.5
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1
    density = 1
    block = '0 1'
  []
  [dummy_material]
    type = GenericConstantMaterial
    block = 2
    prop_names = dummy
    prop_values = 0
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
  csv = true
[]
