[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[Postprocessors]
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  # Emits an informational message because the trapezoidal rule does not match the
  # implicit Euler time integration of the equations
  [u_avg_integrated]
    type = TimeIntegratedPostprocessor
    value = u_avg
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
