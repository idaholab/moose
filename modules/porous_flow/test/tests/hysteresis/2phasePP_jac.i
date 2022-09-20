# Test of derivatives computed in PorousFlow2PhaseHysPP
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
    porous_flow_vars = 'pp0 pp1'
  []
[]

[Variables]
  [pp0]
    initial_condition = 0
  []
  [pp1]
    initial_condition = 1
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
    variable = pp1
  []
  [flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    variable = pp1
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
  [relperm_water]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm_gas]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 1
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlow2PhaseHysPP
    alpha_d = 10.0
    alpha_w = 7.0
    n_d = 1.5
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 12.0
    high_ratio = 0.9
    low_extension_type = quadratic
    high_extension_type = power
    phase0_porepressure = pp0
    phase1_porepressure = pp1
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
