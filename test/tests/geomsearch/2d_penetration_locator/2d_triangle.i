[Mesh]
  file = nonmatching_tri.e
  dim = 2
  construct_side_list_from_node_list = true
  # This test will not work in parallel with ParallelMesh enabled
  # due to a bug in the GeometricSearch system.
  distribution = serial
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./gap_distance]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = leftleft
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = rightright
    value = 1
  [../]
[]

[AuxBCs]
  [./distance]
    type = PenetrationAux
    variable = gap_distance
    boundary = leftright
    paired_boundary = rightleft
  [../]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

