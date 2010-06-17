[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
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
  active = 'example_diff conv diff euler'

  [./example_diff]
    type = ExampleDiffusion
    variable = u
    diffusivity = 0.1
  [../]

  [./conv]
    type = Convection
    variable = u
    velocity_vector = v
  [../]

  [./diff]
    type = Diffusion
    variable = v
  [../]

  [./euler]
    type = ExampleImplicitEuler
    variable = v
    time_coefficient = 20.0
  [../]
[]

[BCs]
  active = 'left_u right_u left_v right_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = '2'
    value = 1

    some_var = v
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '2'
    value = 1
  [../]

[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Transient   # Here we use the Transient Executioner
  perf_log = true
  petsc_options = '-snes_mf_operator'

  num_steps = 20
  dt = 0.5

  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
