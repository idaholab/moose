[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE_VEC
  [../]
[]

[Kernels]
  [./diff]
    type = VectorDiffusion
    variable = u
  [../]

  [./forcing]
    type = VectorBodyForce
    variable = u
    function_x = 'exp(-((x-5)^2+(y-5)^2))'
    function_y = 'exp(-((x-35)^2+(y-35)^2))'
  [../]

  [./dot]
    type = VectorTimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    [./x]
      variable = u
      primary = 3
      secondary = 1
      translation = '40 0 0'
    [../]

    [./y]
      variable = u
      primary = 0
      secondary = 2
      translation = '0 40 0'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 6
  solve_type = NEWTON
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = vector_out
  exodus = true
[]
