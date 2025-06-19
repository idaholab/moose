!include parsed_pp.i

extra = 5

[Postprocessors]
  [parsed]
    pp_symbols = 'L2 L1 extra'
    expression := 'L2/L1'
  []
[]
