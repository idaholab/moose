[Kernels]
  [coupled_uv]
    type = ADCoupledForce
    v = v
    variable = u
  []
  [coupled_vu]
    type = ADCoupledForce
    variable = v
    v = u
  []
[]
