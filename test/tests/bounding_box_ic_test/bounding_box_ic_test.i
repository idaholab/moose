[Mesh]
  file = square.e
  uniform_refine = 3
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.1
      y1 = 0.1
      x2 = 0.6
      y2 = 0.6
      inside = 2.3
      outside = 4.6
    [../]
  [../]
[]

[AuxVariables]
  active = 'u_aux'

  [./u_aux]
    order = FIRST
    family = LAGRANGE

    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.1
      y1 = 0.1
      x2 = 0.6
      y2 = 0.6
      inside = 1.34
      outside = 6.67
    [../]
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
    
