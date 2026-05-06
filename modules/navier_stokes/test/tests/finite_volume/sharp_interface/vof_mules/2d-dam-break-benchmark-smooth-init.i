!include 2d-dam-break-benchmark.i

ic_smoothing_eps = 0.00142875

[Functions]
  [alpha_init]
    expression := '0.25*(1 - tanh((x - ${dam_x})/${ic_smoothing_eps}))*(1 - tanh((y - ${dam_y})/${ic_smoothing_eps}))'
  []
  [pressure_init]
    expression := '-(${rho_l}-${rho_g})*${g}*(${domain_dims_y}-${dam_y})*0.25*(1 - tanh((x - ${dam_x})/${ic_smoothing_eps}))*(1 - tanh((y - ${dam_y})/${ic_smoothing_eps}))'
  []
[]
