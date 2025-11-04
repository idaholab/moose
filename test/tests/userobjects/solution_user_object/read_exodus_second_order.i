[Mesh]
  [file]
    type = FileMeshGenerator
    file = write_exodus_second_order_out.e
  []
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[AuxVariables]
  [temperature_field]
    family = LAGRANGE
    order = SECOND
  []
  [pressure_field]
    family = LAGRANGE
    order = SECOND
  []
[]

[AuxKernels]
  [./nn]
    type = SolutionAux
    variable = temperature_field
    solution = soln
    from_variable = temperature
    #direct = true
  [../]
  [./nn2]
    type = SolutionAux
    variable = pressure_field
    solution = soln
    from_variable = pressure
    #direct = true
  [../]
[]

[UserObjects]
  [./soln]
    type = SolutionUserObject
    mesh = write_exodus_second_order_out.e
    system_variables = 'temperature pressure'
    nodal_variable_order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1.0
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
