# This input file illustrates that PorousFlow can be block-restricted.  That is, porous-flow physics acts only on some blocks (block = 0, in this case), and different physics, in this case diffusion, acts on other blocks (block = 1, in this case).
# Here:
# - the Variable "pressure" exists everywhere, but is governed by PorousFlow only on block = 0, and diffusion on block = 1
# - the Variable "temp" exists only on block = 0, and is governed by PorousFlow there
# - the Variable "temp1" exists only on block = 1, and is governed by diffusion there
# Hence, the PorousFlow Materials only need to be defined on block = 0
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
    bottom_left = '5 -1 -1'
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
  [pressure] # exists over the entire mesh: governed by PorousFlow on block=0, and diffusion on block=1
  []
  [temp]
    block = 0 # only governed by PorousFlow
  []
  [temp1]
    block = 1 # only governed by diffusion
  []
[]

[Kernels]
  [porous_flow_time_derivative]
    type = PorousFlowMassTimeDerivative
    block = 0
    variable = pressure
  []
  [porous_flow_flux]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = pressure
    block = 0
  []
  [porous_flow_heat_time_derivative]
    type = PorousFlowEnergyTimeDerivative
    variable = temp
    block = 0
  []
  [porous_flow_heat_advection]
    type = PorousFlowHeatAdvection
    gravity = '0 0 0'
    variable = temp
    block = 0
  []
  [diffusion_p]
    type = Diffusion
    variable = pressure
    block = 1
  []
  [diffusion_t1]
    type = Diffusion
    variable = temp1
    block = 1
  []
[]

[Modules]
  [FluidProperties]
    [simple_fluid]
      type = SimpleFluidProperties
    []
  []
[]

[Materials] # note these PorousFlow materials are all on block = 0
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
    block = 0
  []
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pressure
    block = 0
  []
  [massfrac]
    type = PorousFlowMassFraction
    block = 0
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
    block = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
    block = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-15 0 0 0 1E-15 0 0 0 1E-15'
    block = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
    block = 0
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1
    density = 1
    block = 0
  []
  [dummy_material]
    type = GenericConstantMaterial
    block = 1
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
