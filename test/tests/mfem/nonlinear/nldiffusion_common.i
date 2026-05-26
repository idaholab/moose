[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[ICs]
  [diffused_ic]
    type = MFEMScalarIC
    coefficient = initial
    variable = concentration
  []
[]

[Functions]
  [initial]
    type = ParsedFunction
    expression = '2 * y + 1'
  []
[]

[BCs]
  [top]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'top'
    coefficient = 3.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = 'bottom'
    coefficient = 1.0
  []
[]

[Kernels]
  [nl]
    type = MFEMNLDiffusionKernel
    variable = concentration
    k_coefficient = concentration
    dk_du_coefficient = 1.0
  []
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointValueSampler
    variable = concentration
    points = '0.5 0.25 0
              0.5 0.50 0
              0.5 0.75 0'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'timestep_end'
  []
[]
