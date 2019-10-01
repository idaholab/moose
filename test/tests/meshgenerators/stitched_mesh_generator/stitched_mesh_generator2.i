[Mesh]
  [./fmg_left]
    type = FileMeshGenerator
    file = left.e
  []

  [./fmg_center]
    type = FileMeshGenerator
    file = center.e
  []

  [./fmg_right]
    type = FileMeshGenerator
    file = right.e
  []

  [./smg]
    type = StitchedMeshGenerator
    inputs = 'fmg_left fmg_center fmg_right'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'right left;
                               right left'
  []
[]

[Variables]
  [./u]
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
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
