!include ../point_value_sampler/point_value_sampler_multi_variable.i

[VectorPostprocessors]
  active = line_sampler
  [line_sampler]
    type = MFEMVariableLineValueSampler
    variables = 'h1_variable hcurl_variable'
    start_point = '-0.375 -0.375 -0.375'
    end_point = '0.375 0.375 0.375'
    num_points = 4
  []
[]

[Outputs]
  active = CSV_line_value
  [CSV_line_value]
    type = CSV
    file_base = OutputData/line_value_sampler_multi_variable
  []
[]
