[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[GlobalParams]
  PorousFlowDictator_UO = dictator
[]

[Variables]
  [./pp]
  [../]
  [./sat]
    [./InitialCondition]
      function = satfunction
      type = FunctionIC
      variable = sat
    [../]
  [../]
[]

[Functions]
  [./satfunction]
    type = ParsedFunction
    value = x
  [../]
[]

[Kernels]
  [./test1]
    type = PorousFlowTestKernel
    variable = pp
  [../]
  [./test2]
    type = PorousFlowTestKernel
    variable = sat
  [../]
[]

[Materials]
  [./relperm0]
    type = PorousFlowMaterialRelativePermeabilityLinear
    phase = 0
    saturation_variable = sat
  [../]
  [./2phase]
    type = PorousFlowMaterial2PhasePS
    phase0_porepressure = pp
    phase1_saturation = sat
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
[]

[Outputs]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp sat'
    number_fluid_phases = 2
    number_fluid_components = 2
  [../]
[]
