[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[Kernels]
  [dt_u]
    type = TimeDerivative
    variable = u
  []
  [diff_u]
    type = Diffusion
    variable = u
  []
  [dt_v]
    type = TimeDerivative
    variable = v
  []
  [diff_v]
    type = MatDiffusion
    variable = v
    diffusivity = 1e-3
  []
[]


[Executioner]
  type = Transient
  num_steps = 2
  automatic_scaling = true
  compute_scaling_once = false
  ignore_variables_for_autoscaling = 'v'
  solve_type = NEWTON
  verbose = true
[]

[Outputs]
  exodus = true
[]
