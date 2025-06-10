!include parsed_pp.i

[Postprocessors]
  [parsed]
    pp_symbols = 'L2 L1'
    expression := 'L2/L1'
  []
[]

[Outputs]
    file_base = parsed_pp_out
[]