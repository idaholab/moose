[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 10
  xmax = 0.304 # Length of test chamber
  ymax = 0.0257 # Test chamber radius
[]

[Variables]
  [./temperature]
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temperature
  [../]
[]

[BCs]
  [./inlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 350 # (K)
  [../]
  [./outlet_temperature]
    type = DirichletBC
    variable = temperature
    boundary = right
    value = 300 # (K)
  [../]
[]

[Materials]
  [./steel]
    type = GenericConstantMaterial
    block = 0
    prop_names = thermal_conductivity
    prop_values = 18 # K: (W/m*K) from wikipedia @296K
  [../]
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
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
