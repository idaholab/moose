# Basic advection with 1 porepressure and temperature as PorousFlow variables
# Constant permeability
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
  [T]
  []
  [P]
  []
[]

[ICs]
  [P]
    type = RandomIC
    variable = P
    min = 2E5
    max = 4E5
  []
  [T]
    type = RandomIC
    variable = T
    min = 300
    max = 900
  []
  [u]
    type = RandomIC
    variable = u
  []
[]

[Kernels]
  [dummy_T]
    type = NullKernel
    variable = T
  []
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
    porous_flow_vars = 'P T'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1E-5
    m = 0.6
    sat_lr = 0.1
  []
[]

[FluidProperties]
  [methane]
    type = MethaneFluidProperties
  []
[]

[Materials]
  [temperature_qp]
    type = PorousFlowTemperature
    temperature = T
  []
  [ppss_qp]
    type = PorousFlow1PhaseP
    porepressure = P
    capillary_pressure = pc
  []
  [fluid_qp]
    type = PorousFlowSingleComponentFluid
    fp = methane
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '5 0 0 0 5 0 0 0 5'
  []
  [relperm_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    s_res = 0.1
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
