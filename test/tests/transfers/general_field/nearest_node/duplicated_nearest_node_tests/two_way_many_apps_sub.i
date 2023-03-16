[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 0.2
  ymax = 0.2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_parent]
  [../]
  [./elemental_from_parent]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [u_elem]
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  # this is done to avoid floating point precision on sending u, with two equidistant points
  [copy_over]
    type = SelfAux
    v = u
    variable = u_elem
  []
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
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
