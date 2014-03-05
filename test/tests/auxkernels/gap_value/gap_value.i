[Mesh]
  file = nonmatching.e
  dim = 2
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in the GeometricSearch system.  See #2121 for more
  # information.
  distribution = serial
[]

[Variables]
  [./u]
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

[AuxBCs]
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]   
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
