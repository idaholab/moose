[Mesh]
  dim = 2
  [./Generation]
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 32
    ny = 32
  [../]
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'example_diff conv diff'

  [./example_diff]
    type = CoefDiffusion
    coef = 0.001
    variable = u
  [../]

  [./conv]
    type = CoupledConvection
    variable = u
    velocity_vector = v
  [../]

  [./diff]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u right_u left_v right_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = '3'
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 1

    some_var = v
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = '3'
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 1
  [../]
[]

[Stabilizers]
  active = 'stab_u'
  [./stab_u]
    type = ConvectionDiffusionSUPG
    variable = u
    coef = 0.001
    x =  1.0
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_supg
  interval = 1
  exodus = true
[]
   
    

