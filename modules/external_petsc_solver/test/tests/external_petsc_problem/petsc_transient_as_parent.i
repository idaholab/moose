[Mesh]
  # It is a mirror of PETSc mesh (DMDA)
  type = PETScDMDAMesh
[]

[AuxVariables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Problem]
  type = ExternalPETScProblem
  sync_variable = u
[]

[Executioner]
  type = Transient
  num_steps = 10
  [./TimeStepper]
    type = ExternalPetscTimeStepper
  [../]
[]

[AuxVariables]
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

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub_app]
    type = TransientMultiApp
    input_files = 'moose_as_sub.i'
    app_type = ExternalPetscSolverTestApp
  [../]
[]

[Transfers]
  [./tosub]
    type = MultiAppShapeEvaluationTransfer
    to_multi_app = sub_app
    source_variable = u
    variable = v
  [../]
[]
