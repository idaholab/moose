[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[Variables]
  [h1]
    family = LAGRANGE
    order = SECOND
  []
  [hcurl]
    family = NEDELEC_ONE
    order = SECOND
  []
  [hdiv]
    family = RAVIART_THOMAS
    order = SECOND
  []
  [l2]
    family = L2_LAGRANGE
    order = FIRST
  []
  [h1_3]
    family = LAGRANGE_VEC
    order = SECOND
  []
  [l2_3]
    family = L2_LAGRANGE_VEC
    order = FIRST
  []
[]

[AuxVariables]
  [l2_aux]
    family = MONOMIAL
    order = CONSTANT
  []
  [l2_3_aux]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VariableSetupTest
  []
[]
