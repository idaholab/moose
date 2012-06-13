[Mesh]
  file = square.e
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  nz = 0
  _dimension = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./saved]
  [../]
  [./bc_saved]
  [../]
  [./accumulated]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    save_in = 'saved accumulated'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./nbc]
    type = NeumannBC
    variable = u
    boundary = right
    value = 1
    save_in = 'bc_saved accumulated'
  [../]
[]

[Postprocessors]
  [./left_flux]
    type = NodalSum
    variable = saved
    boundary = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]

