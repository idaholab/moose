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
    # This Kernel uses "diffusivity" from the active material 
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
  active = empty

  [./empty]
    type = ExampleMaterial
    block = 1
    diffusivity = 0.5
    time_coefficient = 20.0    # New Input paramater use in our Example Material
    
    # Note: These values here are input parameters and in general may or may not correlate with
    # the actual values of the material properties.  That functionality is determined by the
    # computeProperties function within the material.  In this case the input parameters are
    # used directly as the material properties of the same name
  [../]
[]

[Executioner]
  type = TransientHalf   # Here we use our custom Executioner
  perf_log = true
  petsc_options = '-snes_mf_operator'

  num_steps = 40
  dt = 1
  ratio = 0.5
  min_dt = 0.01

  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
