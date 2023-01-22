# Test Corey relative permeability curve by varying saturation over the mesh
# Corey exponent n = 2 for both phases
# No residual saturation in either phase

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [p0]
    initial_condition = 1e6
  []
  [s1]
    family = LAGRANGE
    order = FIRST
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
    type = PorousFlowRelativePermeabilityCorey
    phase = 0
    n = 2
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
  nl_abs_tol = 1e-8
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
