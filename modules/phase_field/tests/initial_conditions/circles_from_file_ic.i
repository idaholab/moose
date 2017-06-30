[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 56
  nz = 0
  xmin = 0
  xmax = 200
  ymin = 0
  ymax = 112
  zmin = 0
  zmax = 0
[]

[Variables]
  [./c]
  [../]
[]

[ICs]
  [./IC_c]
    type = SmoothCircleFromFileIC
    file_name = 'circles.txt'
    invalue = 1
    outvalue = 0
    variable = c
    int_width = 6
  [../]
[]

[Kernels]
  [./c_dot]
    type = TimeDerivative
    variable = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = PJFNK
  num_steps = 0
[]

[Outputs]
  exodus = true
  csv = false
[]
