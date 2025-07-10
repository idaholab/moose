[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
    coefficient = 0.1
  []
  [time]
    type = MFEMTimeDerivativeMassKernel
    variable = u
  []
[]

[BCs]
  [left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = left
    coefficient = 0
  []
  [right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = right
    coefficient = 1
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
  type = MFEMTransient
  end_time = 0.8

  # Use as many different time sequence steppers as we could to test the compositionDT
  [TimeSteppers]
    [ConstDT1]
      type = ConstantDT
      dt = 0.2
    []

    [ConstDT2]
      type = ConstantDT
      dt = 0.1
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [Timesequence1]
      type = TimeSequenceStepper
      time_sequence  = '0  0.25 0.3 0.5 0.8'
    []

    [Timesequence2]
      type = CSVTimeSequenceStepper
      file_name = timesequence.csv
      column_name = time
    []

    [Timesequence3]
      type = ExodusTimeSequenceStepper
      mesh = timesequence.e
    []
  []
[]

[Postprocessors]
  [timestep]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base='mfem_multiple_timesequences'
[]
