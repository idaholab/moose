[Mesh]
  type = MFEMFileMesh
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
  [h1_3]
    family = LAGRANGE_VEC
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

[VectorPostprocessors]
  [point_sample_h1]
    type = MFEMVariablePointValueSampler
    variables = 'h1'
    points = '2.12 0.01 0.125'
  []
  [point_sample_h1_3]
    type = MFEMVariablePointValueSampler
    variables = 'h1_3'
    points = '2.12 0.01 0.125'
  []
  [point_sample_hcurl]
    type = MFEMVariablePointValueSampler
    variables = 'hcurl'
    points = '2.12 0.01 0.125'
  []
  [point_sample_hdiv]
    type = MFEMVariablePointValueSampler
    variables = 'hdiv'
    points = '2.12 0.01 0.125'
  []
  [point_sample_l2]
    type = MFEMVariablePointValueSampler
    variables = 'l2'
    points = '2.12 0.01 0.125'
  []
  [point_sample_l2_3]
    type = MFEMVariablePointValueSampler
    variables = 'l2_3'
    points = '2.12 0.01 0.125'
  []
  [point_sample_l2_aux]
    type = MFEMVariablePointValueSampler
    variables = 'l2_aux'
    points = '2.12 0.01 0.125'
  []
  [point_sample_l2_3_aux]
    type = MFEMVariablePointValueSampler
    variables = 'l2_3_aux'
    points = '2.12 0.01 0.125'
  []
[]

[Outputs]
  file_base = OutputData/MFEMVariableSetupFromMOOSEVariables/var
  csv = true
[]
