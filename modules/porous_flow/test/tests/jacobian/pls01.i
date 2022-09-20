# PorousFlowPiecewiseLinearSink with 1-phase, 1-component
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = RandomIC
    variable = pp
    max = 0
    min = -1
  []
[]

[Kernels]
  [dummy]
    type = TimeDerivative
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 1
    thermal_expansion = 0
    viscosity = 1.1
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
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 2.2 0 0 0 3.3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
[]

[BCs]
  [flux]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'left'
    pt_vals = '-1 -0.5 0'
    multipliers = '1 2 4'
    variable = pp
    fluid_phase = 0
    use_relperm = true
    use_mobility = true
    flux_function = 'x*y'
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 2
[]

[Outputs]
  file_base = pls01
[]
