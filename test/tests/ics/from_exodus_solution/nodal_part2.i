# Use the exodus file for restarting the problem:
# - restart one variable
# - and have one extra variable with IC
#

[Mesh]
  file = out_nodal_part1.e
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = ((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[Variables]
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
    initial_from_file_var = u
    initial_from_file_timestep = 6
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = BoundingBoxIC
      x1 = 0.0
      x2 = 1.0
      y1 = 0.0
      y2 = 1.0
      inside = 3.0
      outside = 1.0
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
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

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_nodal_var_restart
  exodus = true
[]
