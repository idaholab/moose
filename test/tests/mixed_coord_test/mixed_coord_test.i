[Mesh]
  type = MooseMesh
  file = rz_xyz.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./one]
    initial_condition = 1
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
  [./left_middle]
    type = DirichletBC
    variable = u
    boundary = left_middle
    value = 1
  [../]
  [./right_middle]
    type = DirichletBC
    variable = u
    boundary = right_middle
    value = 0
  [../]
[]

[Materials]
  [./empty]
    type = EmptyMaterial
    block = '1 2'
  [../]
[]

[Postprocessors]
  [./volume]
    type = ElementIntegral
    variable = one
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]

[Problem]
  coord_type = 'RZ XYZ'
  block = '1 2'
[]

