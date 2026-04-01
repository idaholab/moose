AD = ''

ic = 5
rate = 1e-4

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [u]
    initial_condition = ${ic}
  []
[]

[Kernels]
  [reaction]
    type = ${AD}Reaction
    variable = u
    rate = ${rate}
  []

  [force]
    type = ${AD}TimeDerivative
    variable = u
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 10
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [u_exact]
    type = ParsedPostprocessor
    expression = '${ic} * exp(-${rate} * t)'
    use_t = true
  []
[]

[Outputs]
  csv = true
  file_base = '${AD}exact_out'
[]
