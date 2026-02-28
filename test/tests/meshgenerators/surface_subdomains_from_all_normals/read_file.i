[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'SAVE_PRE_CRASH.e'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]

[AuxVariables]
  [n_x]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = PorousFlowElementNormal
      component = x
    []
  []
  [n_y]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = PorousFlowElementNormal
      component = y
    []
  []
  [n_z]
    family = MONOMIAL
    order = CONSTANT
    [AuxKernel]
      type = PorousFlowElementNormal
      component = z
    []
  []
[]
