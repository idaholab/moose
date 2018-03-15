[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Variables]
  [./u]
  [../]
  [./v]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./force_u]
    type = BodyForce
    variable = u
    function = 'x*x*x+y*y*y'
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./force_v]
    type = BodyForce
    variable = v
    function = 'x*x*x+y*y*y'
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 'left bottom right top'
    value = 0
  [../]
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 'left bottom right top'
    value = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./l2_difference]
    type = ElementL2Difference
    variable = u
    other_variable = v
  [../]
[]

[Outputs]
  exodus = true
[]
