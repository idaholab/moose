# Basic advection with 1 porepressure as a PorousFlow variable
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
  [P]
  []
[]

[ICs]
  [P]
    type = RandomIC
    variable = P
    min = -1
    max = 1
  []
  [u]
    type = RandomIC
    variable = u
  []
[]

[Kernels]
  [dummy_P]
    type = NullKernel
    variable = P
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
    porous_flow_vars = P
    number_fluid_phases = 1
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
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 3
    density0 = 4
    thermal_expansion = 0
    viscosity = 150.0
  []
[]

[Materials]
  [temperature_qp]
    type = PorousFlowTemperature
  []
  [ppss_qp]
    type = PorousFlow1PhaseP
    porepressure = P
    capillary_pressure = pc
  []
  [simple_fluid_qp]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
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
  [relperm_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
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
    #petsc_options = '-snes_test_display'
    petsc_options_iname = '-snes_type'
    petsc_options_value = ' test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
[]
