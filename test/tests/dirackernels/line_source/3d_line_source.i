[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmin = -2
  xmax =  2
  ymin = -2
  ymax =  2
  ymin = -2
  ymax =  2
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./line_source]
    type = LineSource
    variable = u
    x = 'cos(0.5 * pi * s)'
    y = 'sin(0.5 * pi * s)'
    z = 's'
    strength = 's * s'
    length = 'sqrt(1 + pow(0.5 * pi, 2)) * s'
    constant_names = 'pi'
    constant_expressions = '3.14159265359'
    tmin = 0
    tmax = 1
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
    value = 0
  [../]
[]

[Postprocessors]
  [./pt_v]
    type = PointValue
    variable = u
    point = '0 0 0'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

[]

[Outputs]
  exodus = true
[]
