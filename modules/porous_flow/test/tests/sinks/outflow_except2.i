# Exception testing of PorousFlowOutflowBC.  Note that this input file will produce an error message
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_components = 1
    number_fluid_phases = 1
    porous_flow_vars = pp
  []
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [ppss]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [fluid_props]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = 1
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1 0 0 0 1 0 0 0 1'
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '0.4 0 0 0 0.4 0 0 0 0.4'
  []
[]

[BCs]
  [outflow]
    type = PorousFlowOutflowBC
    boundary = left
    variable = pp
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]
