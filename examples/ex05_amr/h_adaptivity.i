[Mesh]
  dim = 2
  file = square.e

  # Start out with an initial refinement so we have the option to coarsen back
  # if desired.  The mesh with not coarsen more than the initial mesh (after this step)
  uniform_refine = 1
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
    type = Diffusion
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
    value = 10
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
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
  max_r_steps = 7    # Adaptivity Steps

  # The adapativity block
  [./Adaptivity]
    refine_fraction = 0.3
    coarsen_fraction = 0
    max_h_level = 10
    error_estimator = KellyErrorEstimator
    print_changed_info = true
  [../]
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
