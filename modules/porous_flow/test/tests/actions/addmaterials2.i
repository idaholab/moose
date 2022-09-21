# Test that the PorousFlowAddMaterialAction correctly handles the case where
# the at_nodes parameter isn't provided. In this case, only a single material
# is given, and the action must correctly identify if materials should be added
# at the nodes, qps, or even both

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pwater]
    initial_condition = 1e6
  []
  [sgas]
    initial_condition = 0.3
  []
  [temperature]
    initial_condition = 50
  []
[]

[AuxVariables]
  [x0]
    initial_condition = 0.1
  []
  [x1]
    initial_condition = 0.5
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pwater
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sgas
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pwater
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sgas
  []
  [energy_dot]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
  []
  [heat_advection]
    type = PorousFlowHeatAdvection
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pwater sgas temperature'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e-5
    pc_max = 1e7
    sat_lr = 0.1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 1e9
    viscosity = 1e-3
    density0 = 1000
    thermal_expansion = 0
    cv = 2
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 1e9
    viscosity = 1e-4
    density0 = 20
    thermal_expansion = 0
    cv = 1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 50
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = pwater
    phase1_saturation = sgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'x0 x1'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.1
    sum_s_res = 0.11
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
    s_res = 0.01
    sum_s_res = 0.11
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1.0
    density = 125
  []
  [unused]
    type = GenericConstantMaterial
    prop_names = unused
    prop_values = 0
  []
[]

[Executioner]
  type = Transient
  end_time = 1
  nl_abs_tol = 1e-14
[]
