# Checking the Jacobian of Flux-Limited TVD Advection, 1 phase, 1 component, full saturation, using flux_limiter_type = none
# This is quite a heavy test, but we need a fairly big mesh to check the upwinding is happening correctly
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  xmin = 0
  xmax = 1
  ny = 4
  ymin = -1
  ymax = 2
  bias_y = 1.5
  nz = 4
  zmin = 1
  zmax = 2
  bias_z = 0.8
[]

[GlobalParams]
  gravity = '1 2 -0.5'
  PorousFlowDictator = dictator
[]

[Variables]
  [pp]
  []
[]


[ICs]
  [pp]
    variable = pp
    type = RandomIC
    min = 1
    max = 2
  []
[]

[Kernels]
  [flux0]
    type = PorousFlowFluxLimitedTVDAdvection
    variable = pp
    advective_flux_calculator = advective_flux_calculator
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 0.4
    viscosity = 1.1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
  [advective_flux_calculator]
    type = PorousFlowAdvectiveFluxCalculatorSaturated
    flux_limiter_type = None
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.21 0 0  0 1.5 0  0 0 0.8'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
  num_steps = 1
  dt = 1
[]
