[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = cylinder.e
    #parallel_type = replicated
  []

  [./sidesets]
    type = SideSetsFromNormalsGenerator
    input = fmg
    normals = '0  1  0
               0 -1  0'
    fixed_normal = true
    new_boundary = 'front back'
    variance = 0.5
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
  [./front]
    type = DirichletBC
    variable = u
    boundary = front
    value = 0
  [../]
  [./back]
    type = DirichletBC
    variable = u
    boundary = back
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
