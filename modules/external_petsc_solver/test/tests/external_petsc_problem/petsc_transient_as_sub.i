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
