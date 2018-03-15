[Mesh]
  file = nonmatching.e
  dim = 2
  # This test will not work in parallel with DistributedMesh enabled
  # due to a bug in the GeometricSearch system.  See #2121 for more
  # information.
  parallel_type = replicated
[]

[Variables]
  [./u]
    block = 'left right'
  [../]
[]

[AuxVariables]
  [./gap_value]
    block = left
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 'leftbottom rightbottom'
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = 'lefttop righttop'
    value = 1
  [../]
[]

[AuxKernels]
  [./gap_value_aux]
    type = GapValueAux
    variable = gap_value
    boundary = leftright
    paired_variable = u
    paired_boundary = rightleft
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
