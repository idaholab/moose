[Mesh]
  [gmg]
    type = DistributedRectilinearMeshGenerator
    dim = 2
    nx = 20
    ny = 21
    partition = square
  []
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v]
  [../]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./cf]
    type = CoupledForce
    coef = 10000
    variable = u
    v=v
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
  num_steps = 10
  dt = 0.2

  solve_type = 'PJFNK'

  fixed_point_max_its = 10
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-9
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-12

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [./picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  [../]
[]

[MultiApps]
  [./sub_app]
    type = TransientMultiApp
    input_files = 'petsc_transient_as_sub.i'
    app_type = ExternalPetscSolverApp
    library_path = '../../../../external_petsc_solver/lib'
  [../]
[]

[Transfers]
  [./fromsub]
    type = MultiAppShapeEvaluationTransfer
    from_multi_app = sub_app
    source_variable = u
    variable = v
  [../]
[]
