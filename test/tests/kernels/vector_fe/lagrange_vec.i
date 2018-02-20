# This example reproduces the libmesh vector_fe example 1 results

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = -1
  ymin = -1
  elem_type = QUAD9
[]

[Variables]
  [./u]
    family = LAGRANGE_VEC
    order = SECOND
  [../]
[]

[Kernels]
  [./diff]
    type = LinearVectorPoisson
    variable = u
    x_exact_sln = 'x_exact_sln'
    y_exact_sln = 'y_exact_sln'
  [../]
[]

[BCs]
  [./bnd]
    type = LinearVectorPenaltyDirichletBC
    variable = u
    x_exact_sln = 'x_exact_sln'
    y_exact_sln = 'y_exact_sln'
    penalty = 1e10
    boundary = 'left right top bottom'
  [../]
[]

[Functions]
  [./x_exact_sln]
    type = ParsedFunction
    value = 'cos(.5*pi*x)*sin(.5*pi*y)'
  [../]
  [./y_exact_sln]
    type = ParsedFunction
    value = 'sin(.5*pi*x)*cos(.5*pi*y)'
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'LINEAR'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  petsc_options = '-ksp_converged_reason'
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    execute_on = 'initial timestep_begin linear failed'
  [../]
[]
