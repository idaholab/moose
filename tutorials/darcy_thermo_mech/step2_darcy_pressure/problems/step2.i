[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 10
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Half inner diameter of test chamber
[]

[Variables]
  [./pressure]
  [../]
[]

[Kernels]
  [./darcy_pressure]
    type = DarcyPressure
    variable = pressure
    viscosity = 7.98e-4 # (Pa*s) Water at 30 degrees C (Wikipedia)
    permeability = 0.8451e-9 # (m^2) 1mm balls.  From paper
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

