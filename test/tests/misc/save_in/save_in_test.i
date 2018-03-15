[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
  [./diag_saved]
  [../]
  [./bc_diag_saved]
  [../]
  [./saved_dirichlet]
  [../]
  [./diag_saved_dirichlet]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    save_in = 'saved accumulated saved_dirichlet'
    diag_save_in = 'diag_saved diag_saved_dirichlet'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
    save_in = saved_dirichlet
    diag_save_in = diag_saved_dirichlet
  [../]
  [./nbc]
    type = NeumannBC
    variable = u
    boundary = right
    value = 1
    save_in = 'bc_saved accumulated'
    diag_save_in = bc_diag_saved
  [../]
[]

[Postprocessors]
  [./left_flux]
    type = NodalSum
    variable = saved
    boundary = 1
  [../]
  [./saved_norm]
    type = NodalL2Norm
    variable = saved
    execute_on = timestep_end
    block = 0
  [../]
  [./saved_dirichlet_norm]
    type = NodalL2Norm
    variable = saved_dirichlet
    execute_on = timestep_end
    block = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  file_base = out
  exodus = true
[]
