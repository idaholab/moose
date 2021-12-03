# Exception testing of PorousFlowPorosityLinear: demonstrating that an error is thrown if there are missing Materials
[GlobalParams]
  PorousFlowDictator = dictator
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  kernel_coverage_check = false
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
  []
  [disp]
  []
[]

[Kernels]
  [pp]
    type = Diffusion
    variable = pp
  []
[]

[Materials]
  [ps]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [temperature]
    type = PorousFlowTemperature
  []
  [pf]
    type = PorousFlowEffectiveFluidPressure
  []
  [volstrain]
    type = PorousFlowVolumetricStrain
    displacements = pp
  []
  [porosity]
    type = PorousFlowPorosityLinear
    porosity_ref = 0.1
  []
  [total_strain]
    type = ComputeSmallStrain
    displacements = disp
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
[]
