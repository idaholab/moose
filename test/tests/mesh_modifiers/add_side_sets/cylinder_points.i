[Mesh]
  type = FileMesh
  file = cylinder.e
  # This MeshModifier currently only works with SerialMesh.
  # For more information, refer to #2129.
  distribution = serial
[]

# Mesh Modifiers
[MeshModifiers]
  [./add_side_sets]
    type = SideSetsFromPoints
    points = '0   0  0.5
              0.1 0  0
              0   0 -0.5'
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


