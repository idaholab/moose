# Test van Genuchten relative permeability curve by varying saturation over the mesh
# van Genuchten exponent m = 0.4 for both phases
# Phase 0 residual saturation s0r = 0.1
# Phase 1 residual saturation s1r = 0.2

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
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
  [kr0aux]
    family = MONOMIAL
    order = CONSTANT
  []
  [kr1aux]
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
  [kr0]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 0
    variable = kr0aux
  []
  [kr1]
    type = PorousFlowPropertyAux
    property = relperm
    phase = 1
    variable = kr1aux
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
    type = PorousFlowCapillaryPressureConst
    pc = 0
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
    m = 0.4
    s_res = 0.1
    sum_s_res = 0.3
  []
  [kr1]
    type = PorousFlowRelativePermeabilityVG
    phase = 1
    m = 0.4
    s_res = 0.2
    sum_s_res = 0.3
    wetting = false
  []
[]

[VectorPostprocessors]
  [vpp]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    variable = 's0aux s1aux kr0aux kr1aux'
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 20
    sort_by = id
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-7
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
