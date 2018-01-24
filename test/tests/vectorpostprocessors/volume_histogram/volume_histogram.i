[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 200
  xmin = -5
  xmax = 5
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = FunctionIC
      function = 'x<2&x>-2'
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[BCs]
  [./all]
    type = DirichletBC
    variable = c
    boundary = 'left right'
    value = 0
  [../]
[]

[VectorPostprocessors]
  [./histo]
    type = VolumeHistogram
    variable = c
    min_value = 0
    max_value = 1.1
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
  solve_type = PJFNK
[]

[Outputs]
  execute_on = 'initial timestep_end'
  csv = true
[]
