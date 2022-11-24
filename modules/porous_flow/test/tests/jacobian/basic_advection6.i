# Basic advection with 2 porepressure as PorousFlow variables
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [u]
  []
  [P0]
  []
  [P1]
  []
[]

[ICs]
  [P0]
    type = RandomIC
    variable = P0
    min = -1
    max = 0
  []
  [P1]
    type = RandomIC
    variable = P1
    min = 0
    max = 1
  []
  [u]
    type = RandomIC
    variable = u
  []
[]

[Kernels]
  [dummy_P0]
    type = NullKernel
    variable = P0
  []
  [dummy_P1]
    type = NullKernel
    variable = P1
  []
  [u_advection]
    type = PorousFlowBasicAdvection
    variable = u
    phase = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'P0 P1'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1
    m = 0.6
    sat_lr = 0.1
  []
[]

[FluidProperties]
  [simple_fluid0]
    type = SimpleFluidProperties
    bulk_modulus = 3
    density0 = 4
    thermal_expansion = 0
    viscosity = 150.0
  []
  [simple_fluid1]
    type = SimpleFluidProperties
    bulk_modulus = 4
    density0 = 3
    thermal_expansion = 0
    viscosity = 130.0
  []
[]

[Materials]
  [temperature_qp]
    type = PorousFlowTemperature
  []
  [ppss_qp]
    type = PorousFlow2PhasePP
    phase0_porepressure = P0
    phase1_porepressure = P1
    capillary_pressure = pc
  []
  [simple_fluid0_qp]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid0
    phase = 0
  []
  [simple_fluid1_qp]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid1
    phase = 1
  []
  [effective_fluid_pressure]
    type = PorousFlowEffectiveFluidPressure
  []
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
    fluid = true
    biot_coefficient = 0.5
    solid_bulk = 1
  []
  [permeability]
    type = PorousFlowPermeabilityKozenyCarman
    poroperm_function = kozeny_carman_phi0
    k0 = 5
    m = 2
    n = 2
    phi0 = 0.1
  []
  [relperm0_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.1
    sum_s_res = 0.1
  []
  [relperm1_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
    s_res = 0.0
    sum_s_res = 0.1
  []
  [darcy_velocity_qp]
    type = PorousFlowDarcyVelocityMaterial
    gravity = '0.25 0 0'
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options = '-snes_test_display'
    petsc_options_iname = '-snes_type'
    petsc_options_value = ' test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
[]
