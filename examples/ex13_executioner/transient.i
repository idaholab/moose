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
  active = 'example'

  [./example]
    type = ExampleMaterial
    block = 1
    diffusivity = 0.5
    time_coefficient = 20.0 
  [../]
[]

[Executioner]
  # Each simulation has a single executioner so we don't specify them
  # in subblocks.  Instead we just change the type inside the Executioner.

  type = TransientHalf   # Here we use our custom Executioner
  perf_log = true
  petsc_options = '-snes_mf_operator'

  num_steps = 40
  dt = 1
  ratio = 0.5
  min_dt = 0.01
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
