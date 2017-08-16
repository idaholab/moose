[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./u_diffusion]
    type = Diffusion
    variable = u
  [../]
  [./u_dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[Postprocessors]
  [./u_position]
    type = CInterfacePosition
    variable = u
    RefVal = 0.8
    execute_on = linear
    direction_index = 0
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = .005
  solve_type = NEWTON
  petsc_options_iname = -pc_type
  petsc_options_value = lu
  end_time = 1
[]

[Outputs]
  file_base = out
  exodus = true
[]

[ICs]
  [./u_ic]
    y2 = 0
    y1 = 0
    inside = 1
    x2 = .5
    variable = u
    x1 = 0
    type = BoundingBoxIC
  [../]
[]
