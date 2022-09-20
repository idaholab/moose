# Test of derivatives computed in PorousFlowHystereticRelativePermeability classes along first-order curve
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '-1 0 0'
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 2
    number_fluid_components = 2
    porous_flow_vars = 'pp0 sat1'
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 10.0
    m = 0.33
  []
[]

[Variables]
  [pp0]
  []
  [sat1]
    initial_condition = 0.5
  []
[]

[Kernels]
  [mass_conservation0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp0
  []
  [flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    variable = pp0
  []
  [mass_conservation1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = sat1
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = sat1
  []
[]

[AuxVariables]
  [massfrac_ph0_sp0]
    initial_condition = 1
  []
  [massfrac_ph1_sp0]
    initial_condition = 0
  []
[]

[FluidProperties]
  [simple_fluid_0]
    type = SimpleFluidProperties
    bulk_modulus = 10
    viscosity = 1
  []
  [simple_fluid_1]
    type = SimpleFluidProperties
    bulk_modulus = 1
    viscosity = 3
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [temperature]
    type = PorousFlowTemperature
  []
  [massfrac]
    type = PorousFlowMassFraction
    mass_fraction_vars = 'massfrac_ph0_sp0 massfrac_ph1_sp0'
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid_0
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid_1
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0  0 1 0  0 0 1'
  []
  [pc_calculator]
    type = PorousFlow2PhasePS
    capillary_pressure = pc
    phase0_porepressure = pp0
    phase1_saturation = sat1
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
    initial_order = 1
    previous_turning_points = 0.3
  []
  [relperm_liquid]
    type = PorousFlowHystereticRelativePermeabilityLiquid
    phase = 0
    S_lr = 0.1
    S_gr_max = 0.2
    m = 0.9
    liquid_modification_range = 0.9
  []
  [relperm_gas]
    type = PorousFlowHystereticRelativePermeabilityGas
    phase = 1
    S_lr = 0.1
    S_gr_max = 0.2
    m = 0.9
    gamma = 0.33
    k_rg_max = 0.8
    gas_low_extension_type = linear_like
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options = '-snes_check_jacobian'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]
