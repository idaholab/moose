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

[Functions]
  [dts]
    type = PiecewiseLinear
    x = '0   0.85 2'
    y = '0.2 0.15  0.2'
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
  l_tol = 1e-8
  l_max_its = 100
[]

[Executioner]
  type = MFEMTransient
  end_time = 0.8
  # Use as many different time steppers as we could to test the compositionDT,
  # SolutionTimeAdaptiveDT give slightly different dt per run, set rel_err = 1e-2
  # to ensure the test won't fail due to the small difference in the high-digit.
  [TimeSteppers]
    [ConstDT1]
      type = ConstantDT
      dt = 0.2
    []

    [FunctionDT]
      type = FunctionDT
      function = dts
    []

    [LogConstDT]
      type = LogConstantDT
      log_dt = 0.2
      first_dt = 0.1
    []

    [IterationAdapDT]
      type = IterationAdaptiveDT
      dt = 0.5
    []

    [Timesequence]
      type = TimeSequenceStepper
      time_sequence  = '0  0.25 0.3 0.5 0.8'
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
  file_base='mfem_multiple_timesteppers'
[]
