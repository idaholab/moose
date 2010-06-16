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
  active = 'diff_u conv diff_v'

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./conv]
    type = Convection
    variable = u
    velocity_vector = v
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  active = 'left_u right_u_dirichlet left_v right_v'

  [./left_u]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right_u_dirichlet]
    type = CoupledDirichletBC
    variable = u
    boundary = '2'
    value = 2

    some_var = v
  [../]

  # Note: This BC is not active in this input file
  [./right_u_neumann]
    type = CoupledNeumannBC
    variable = u
    boundary = '2'
    value = 2
    
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
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
[]
   
    
