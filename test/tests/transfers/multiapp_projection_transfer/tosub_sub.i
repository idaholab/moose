[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3
  ymin = 0
  ymax = 3
  nx = 3
  ny = 3
[]

[Variables]
  [./v]
  [../]
[]

#[AuxVariables]
#  [./u_nodal]
#  [../]
#  [./u_elemental]
#    order = CONSTANT
#    family = MONOMIAL
#  [../]
#  [./x_elemental]
#    order = CONSTANT
#    family = MONOMIAL
#  [../]
#  [./x_nodal]
#  [../]
#[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = v
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'NEWTON'
  print_linear_residuals = true
[]

[Outputs]
  exodus = true
  output_on = 'initial timestep_end'
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear linear'
  [../]
[]
