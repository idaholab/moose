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
  active = 'diffused'   # Note the active list here

  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]

# This variable is not active in the list above 
# therefore it is not used in the simulation
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

# This example applies DirichletBCs to all four sides of our square domain
[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = diffused
    boundary = '1'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = diffused
    boundary = '2'
    value = 1
  [../]
[]
  
    
