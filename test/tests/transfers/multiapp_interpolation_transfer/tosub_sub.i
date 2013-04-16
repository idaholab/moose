[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_master]
  [../]
  [./elemental_from_master]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./radial_from_master]
  [../]
  [./radial_elemental_from_master]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./disp_x]
    initial_condition = -0.2
  [../]
  [./disp_y]
  [../]
  [./displaced_target_from_master]
  [../]
  [./displaced_source_from_master]
  [../]
  [./elemental_from_master_elemental]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./nodal_from_master_elemental]
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
  type = Transient
  num_steps = 1
  dt = 1
  petsc_options = '-snes_mf_operator -ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

