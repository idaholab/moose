[Mesh]
  type = FileMesh
  file = meshed_gap.e
  dim = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./gap_distance]
  [../]
  [./gap_value]
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
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[AuxKernels]
  [./penetration]
    type = PenetrationAux
    variable = gap_distance
    boundary = 2
    paired_boundary = 3
  [../]
  [./gap_value]
    type = GapValueAux
    variable = gap_value
    boundary = 2
    paired_variable = u
    paired_boundary = 3
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
