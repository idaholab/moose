[Mesh]
  type = MFEMMesh
  file = ../mesh/cylinder-hex-q2.gen
[]

[Problem]
  type = MFEMProblem
  solve = false
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
    basis = GaussLegendre
  []
[]

[Variables]
  [h1_scalar]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [l2_scalar]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [height]
    type = ParsedFunction
    expression = 'z'
  []
[]

[ICs]
  [l2_scalar_ic]
    type = MFEMScalarIC
    variable = l2_scalar
    coefficient = 2.0
  []
  [h1_scalar_ic]
    type = MFEMScalarIC
    variable = h1_scalar
    coefficient = height
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [line_sample_h1_scalar]
    type = MFEMLineValueSampler
    variable = 'h1_scalar'
    start_point = '-1 0 -0.5'
    end_point = '1 0 0.5'
    num_points = 101
  []
  [line_sample_l2_scalar]
    type = MFEMLineValueSampler
    variable = 'l2_scalar'
    start_point = '-1 0 -0.5'
    end_point = '1 0 0.5'
    num_points = 101
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/ScalarIC/scalar_ic
  []
[]
