[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    variable = u
    value = 2
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./u_avg]
    type = ElementAverageValue
    variable = u
    execute_on = 'initial timestep_end'
  [../]

  [./scaled_u]
    type = ScalePostprocessor
    value = u_avg
    scaling_factor = 2
    execute_on = 'initial timestep_end'
  [../]

  [./scaled_scaled_u]
    type = ScalePostprocessor
    value = scaled_u
    scaling_factor = 2
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
