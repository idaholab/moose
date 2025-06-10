!include parsed_pp.i

[Postprocessors]
  [parsed]
    pp_symbols = 'L2 L1'
    pp_names := 'L2:norm L1_norm'
    expression := 'L2/L1'
  []
[]

[Outputs]
    file_base = parsed_pp_out
[]
