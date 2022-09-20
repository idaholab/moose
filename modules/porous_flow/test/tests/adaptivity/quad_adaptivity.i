[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[Adaptivity]
  marker = marker
  max_h_level = 1
  [Markers]
    [marker]
      type = UniformMarker
      mark = REFINE
    []
  []
[]

[GlobalParams]
  PorousFlowDictator = 'dictator'
[]

[Variables]
  [pp]
    initial_condition = '0'
  []
[]

[Kernels]
  [mass]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
  [flux]
    type = PorousFlowAdvectiveFlux
    variable = pp
    gravity = '0 0 0'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = pp
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = pp
    boundary = 'right'
    value = 0
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
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1.2
    density0 = 1
    viscosity = 1
    thermal_expansion = 0
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = 'pp'
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
  [porosity]
    type = PorousFlowPorosityConst
    porosity = '0.1'
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-3 0 0 0 1e-3 0 0 0 1e-3'
  []
  [relperm]
    type = PorousFlowRelativePermeabilityConst
    phase = 0
  []
[]

[Postprocessors]
  [numdofs]
    type = NumDOFs
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  end_time = 4
  dt = 1
  solve_type = Newton
  nl_abs_tol = 1e-12
[]

[Outputs]
  execute_on = 'final'
  exodus = true
  perf_graph = true
  show = pp
[]
