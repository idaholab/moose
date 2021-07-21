# Testing PorousFlowPorosityLinear produces correct values:
# porosity = porosity_ref + P_coeff * (P - P_ref) + T_coeff * (T - T_ref) + epv_coeff * (epv - epv_coeff)
#          = 0.5 + 2 * (1 - 0.5) + 0.5 * (2 - -3) + 4 * (3 - 2.5)
#          = 6
[GlobalParams]
  PorousFlowDictator = dictator
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 1
    number_fluid_components = 1
    porous_flow_vars = pp
  []
[]

[Variables]
  [pp]
    initial_condition = 1
  []
  [T]
    initial_condition = 2
  []
  [disp]
  []
[]

[ICs]
  [disp]
    type = FunctionIC
    variable = disp
    function = '3 * x'
  []
[]

[Kernels]
  [pp]
    type = TimeDerivative
    variable = pp
  []
  [T]
    type = TimeDerivative
    variable = T
  []
  [disp]
    type = TimeDerivative
    variable = disp
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [porosity]
    type = PorousFlowPropertyAux
    variable = porosity
    property = porosity
  []
[]

[Postprocessors]
  [porosity]
    type = PointValue
    point = '0 0 0'
    variable = porosity
  []
[]

[Materials]
  [ps]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [temperature]
    type = PorousFlowTemperature
    temperature = T
  []
  [pf]
    type = PorousFlowEffectiveFluidPressure
  []
  [total_strain]
    type = ComputeSmallStrain
    displacements = disp
  []
  [volstrain]
    type = PorousFlowVolumetricStrain
    displacements = disp
  []
  [porosity]
    type = PorousFlowPorosityLinear
    porosity_ref = 0.5
    P_ref = 0.5
    P_coeff = 2.0
    T_ref = -3.0
    T_coeff = 0.5
    epv_ref = 2.5
    epv_coeff = 4.0
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]

[Outputs]
  csv = true
[]
