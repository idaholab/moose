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
  [line_sample_h1]
    type = MFEMLineValueSampler
    variable = 'h1'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_h1_3]
    type = MFEMLineValueSampler
    variable = 'h1_3'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_hcurl]
    type = MFEMLineValueSampler
    variable = 'hcurl'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_hdiv]
    type = MFEMLineValueSampler
    variable = 'hdiv'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_l2]
    type = MFEMLineValueSampler
    variable = 'l2'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_l2_3]
    type = MFEMLineValueSampler
    variable = 'l2_3'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_l2_aux]
    type = MFEMLineValueSampler
    variable = 'l2_aux'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
  [line_sample_l2_3_aux]
    type = MFEMLineValueSampler
    variable = 'l2_3_aux'
    start_point = '2.12 0.01 -2.37'
    end_point = '2.12 0.01 2.62'
    num_points = 11
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = OutputData/MFEMVariableSetupFromMOOSEVariables/var
  csv = true
[]
