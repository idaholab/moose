# POD Reduced Basis Surrogate

!plot scatter
  filename=examples/surrogates/gold/poly_chaos_order_uniform_results.csv
  data=[{'x':'no_modes', 'y':'mean', 'name':'T&#773; &#956; error'},
        {'x':'no_modes', 'y':'max', 'name':'T&#773; &#963; error'}]
  layout={'xaxis':{'type':'log','title':'Number of basis functions'},
          'yaxis':{'type':'log','title':'Relative Error'}}
