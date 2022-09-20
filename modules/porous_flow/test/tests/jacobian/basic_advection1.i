# Basic advection with no PorousFlow variables
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
[]

[AuxVariables]
  [P]
  []
[]

[ICs]
  [P]
    type = FunctionIC
    variable = P
    function = '2*(1-x)'
  []
  [u]
    type = RandomIC
    variable = u
  []
[]

[Kernels]
  [u_advection]
    type = PorousFlowBasicAdvection
    variable = u
    phase = 0
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = ''
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2e9
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
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '5 0 0 0 5 0 0 0 5'
  []
  [relperm_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 0
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
