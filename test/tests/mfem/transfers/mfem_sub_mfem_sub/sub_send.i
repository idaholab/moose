[Mesh]
  type = MFEMMesh
  file = ../../mesh/square.msh
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
  [send]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [back]
    type = MFEMScalarDirichletBC
    variable = send
    boundary = 1
    coefficient = 1.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = send
    boundary = 2
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = send
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
[]

[Executioner]
  type = MFEMSteady
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'send'
    start_point = '0 0 0'
    end_point = '1 1 0.1'
    num_points = 101
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/DiffusionSendApp/send
  []
[]
