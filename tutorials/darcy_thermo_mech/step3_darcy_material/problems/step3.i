[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 10
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Test chamber radius
[]

[Variables]
  [./pressure]
  [../]
[]

[Kernels]
  [./darcy_pressure]
    type = DarcyPressure
    variable = pressure
    # No parameters necessary because the values will come from the material system
  [../]
[]

[BCs]
  [./inlet]
    type = DirichletBC
    variable = pressure
    boundary = left
    value = 205778.43 # (Pa) From Figure 2 from paper.  First dot for 1mm balls.
  [../]
  [./outlet]
    type = DirichletBC
    variable = pressure
    boundary = right
    value = 0 # (Pa) Gives the correct pressure drop from Figure 2 for 1mm balls
  [../]
[]

[Materials]
  [./column]
    type = PackedColumn
    block = 0
    ball_radius = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
