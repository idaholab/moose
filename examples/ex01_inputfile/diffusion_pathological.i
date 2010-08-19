[Mesh]
  dim = 2
  file = square.e
  uniform_refine = 4
[]

# Note: This output block is out of it's normal place (should be at the bottom)
[Output]
  file_base = out
  interval = 1
  exodus = true
[]

# Note: The executioner is out of it's normal place (should be just about the output block)
[Executioner]
	type = Steady
	perf_log = true
	petsc_options = '-snes_mf_operator'
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Variables]
  active = 'u'   # Note the active list here

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

# This variable is not active in the list above 
# therefore it is not used in the simulation
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

# This example applies DirichletBCs to all four sides of our square domain
[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = '2'
    value = 1
  [../]
[]
  
    
