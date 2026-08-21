!include ../kernels/heattransfer.i

[ProblemComposers]
  [default_transient]
    type = MFEMTimeDependentWeakFormProblemComposer
  []
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMVariableLineValueSampler
    variable = 'temperature'
    start_point = '2.125 0 -2.375'
    end_point = '2.125 0 2.625'
    num_points = 101
  []
[]


[Outputs]
  [CSV]
    type = CSV
    execute_on = 'timestep_end'
    file_base = OutputData/HeatTransfer/heattransfer
  []
[]
