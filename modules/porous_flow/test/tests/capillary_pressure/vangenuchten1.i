# Test van Genuchten relative permeability curve by varying saturation over the mesh
# van Genuchten exponent m = 0.5 for both phases
# No residual saturation in either phase

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 500
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [p0]
    initial_condition = 1e6
  []
  [s1]
  []
[]

[AuxVariables]
  [s0aux]
    family = MONOMIAL
    order = CONSTANT
  []
  [s1aux]
    family = MONOMIAL
    order = CONSTANT
  []
  [p0aux]
    family = MONOMIAL
    order = CONSTANT
  []
  [p1aux]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [s0]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 0
    variable = s0aux
  []
  [s1]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 1
    variable = s1aux
  []
  [p0]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 0
    variable = p0aux
  []
  [p1]
    type = PorousFlowPropertyAux
    property = pressure
    phase = 1
    variable = p1aux
  []
[]

[Functions]
  [s1]
    type = ParsedFunction
    expression = x
  []
[]

[ICs]
  [s1]
    type = FunctionIC
    variable = s1
    function = s1
  []
[]

[Kernels]
  [p0]
    type = Diffusion
    variable = p0
  []
  [s1]
    type = Diffusion
    variable = s1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'p0 s1'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    alpha = 1e-5
    m = 0.5
    sat_lr = 0.1
    log_extension = false
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow2PhasePS
    phase0_porepressure = p0
    phase1_saturation = s1
    capillary_pressure = pc
  []
  [kr0]
    type = PorousFlowRelativePermeabilityVG
    phase = 0
    m = 0.5
  []
  [kr1]
    type = PorousFlowRelativePermeabilityCorey
    phase = 1
    n = 2
  []
[]

[VectorPostprocessors]
  [vpp]
    type = LineValueSampler
    variable = 's0aux s1aux p0aux p1aux'
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 500
    sort_by = id
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-6
[]

[BCs]
  [sleft]
    type = DirichletBC
    variable = s1
    value = 0
    boundary = left
  []
  [sright]
    type = DirichletBC
    variable = s1
    value = 1
    boundary = right
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]
