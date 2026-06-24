!include reference_residual.i

[Postprocessors]
  active = 'res_calls elapsed'
  [res_calls]
    type = PerfGraphData
    section_name = "ReferenceResidualProblem::computeResidualInternal"
    data_type = calls
  []
  [elapsed]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  []
[]

[Outputs]
  csv = true
  file_base = reference_residual_perfgraph_out
[]
