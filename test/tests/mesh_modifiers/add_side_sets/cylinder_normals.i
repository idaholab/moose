[Mesh]
  type = FileMesh
  file = cylinder.e
[]

# Mesh Modifiers
[MeshModifiers]
  [./add_side_sets]
    type = SideSetsFromNormals
    normals = '0  0  1
               0  1  0
               0  0 -1'
    boundary = 'top side bottom'
  [../]
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
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]


