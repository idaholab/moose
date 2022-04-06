# This test was introduced for Issue #804 which saw data corruption
# during NearestNodeTransfer when running in parallel

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  parallel_type = replicated
[]

[Variables]
  [./u]
     order = FIRST
     family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./from_sub]
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

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 1.0 0.0'
    input_files = parallel_sub.i
    execute_on = 'timestep_end'
   [../]
[]

[Transfers]
# Surface to volume data transfer
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = u
    variable = from_sub
    execute_on = 'timestep_end'
  [../]
[]
