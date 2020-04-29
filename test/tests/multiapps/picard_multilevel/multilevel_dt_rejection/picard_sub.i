[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./v2]
  [../]
  [./v3]
  [../]
  [./w]
  [../]
[]

[AuxKernels]
  [./set_w]
    type = NormalizationAux
    variable = w
    source_variable = v
    normal_factor = 0.1
  [../]
[]

[Kernels]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./coupled_force]
    type = CoupledForce
    variable = v
    v = v2
  [../]
  [./coupled_force2]
    type = CoupledForce
    variable = v
    v = v3
  [../]
  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = left
    function = func
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Functions]
  [func]
    type = ParsedFunction
    value = 'if(t < 2.5, 1, 1 / t)'
  []
[]

[Postprocessors]
  [./picard_its]
    type = NumPicardIterations
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  picard_max_its = 2 # deliberately make it fail at 2 to test the time step rejection behavior
  nl_rel_tol = 1e-5 # loose enough to force multiple Picard iterations on this example
  nl_abs_tol = 1e-9
  picard_rel_tol = 1e-8
  picard_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub2]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = picard_sub2.i
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./v_to_v3]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = sub2
    source_variable = v
    variable = v3
  [../]
  [./w]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = sub2
    source_variable = w
    variable = w
  [../]
[]
