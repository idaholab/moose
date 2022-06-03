[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 2
[]

[Adaptivity]
  marker = marker
  max_h_level = 2
  cycles_per_step = 2
  [./Indicators]
    [./error]
      type = GradientJumpIndicator
      variable = u
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorFractionMarker
      coarsen = 0.4
      refine = 0.5
      indicator = error
    [../]
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
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

[Problem]
  type = LevelSetProblem
[]

[Executioner]
  type = Transient
  dt = 0.02
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    input_files = 'sub.i'
    execute_on = TIMESTEP_END
  [../]
[]

[Transfers]
  [./marker_to_sub]
    type = LevelSetMeshRefinementTransfer
    to_multi_app = sub
    source_variable = marker
    variable = marker
    check_multiapp_execute_on = false
  [../]
[]

[Outputs]
  hide = u
  exodus = true
[]
