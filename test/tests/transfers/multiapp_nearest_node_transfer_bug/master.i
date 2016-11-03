[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 150
  ny = 150
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
  [./pid]
    order = constant
    family = monomial
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

  # Preconditioned JFNK (default)
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
    input_files = sub.i
    execute_on = 'timestep_end'
   [../]
[]

[Transfers]
# Surface to volume data transfer
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = u
    variable = from_sub
    execute_on = 'timestep_end'
  [../]
[]

[AuxKernels]
  [./pid]
    type = ProcessorIDAux
    variable = pid
  [../]
[]
