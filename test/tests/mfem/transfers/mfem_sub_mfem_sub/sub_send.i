[Mesh]
  type = MFEMFileMesh
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
  [transfer_var]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [back]
    type = MFEMScalarDirichletBC
    variable = transfer_var
    boundary = 1
    coefficient = 1.0
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = transfer_var
    boundary = 2
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = transfer_var
  []
[]


[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
    l_tol = 1e-16
    l_max_its = 1000
  []
[]

[Executioner]
  type = MFEMSteady
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMVariableLineValueSampler
    variable = 'transfer_var'
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
