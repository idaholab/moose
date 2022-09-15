# Check PorousFlowUnsaturated throws an error when stabilization = none

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[PorousFlowUnsaturated]
  porepressure = pp
  dictator_name = dictator
  fp = simple_fluid
  stabilization = none
[]

[Variables]
  [pp]
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-13 0 0  0 1E-13 0  0 0 1E-13'
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.3
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]
