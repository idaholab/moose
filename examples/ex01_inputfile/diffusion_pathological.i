[Mesh]
  file = square.e
  uniform_refine = 4
[]

# Note: This output block is out of its normal place (should be at the bottom)
[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

# Note: The executioner is out of its normal place (should be just about the output block)
[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Variables]
  active = 'diffused'   # Note the active list here
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]

# This variable is not active in the list above
# therefore it is not used in the simulation
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

# This example applies DirichletBCs to all four sides of our square domain
[BCs]
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
